#include "Compiler.h"
#include "VirtualMachine.h"
#include "Lexer.h"
#include <vector>

// Compiler constructor
Compiler::Compiler(std::vector<Token>& toks)
    : tokens(toks), pos(0), labelCount(0), jumpCount(0), labelCounter(0) {}

// Private helper methods
Token& Compiler::current() {
    return tokens[pos];
}

Token& Compiler::peek(int offset) {
    if (pos + offset >= tokens.size()) return tokens[tokens.size() - 1];
    return tokens[pos + offset];
}

void Compiler::advance() {
    if (pos < tokens.size() - 1) pos++;
}

bool Compiler::match(TokenType type) {
    if (current().type == type) {
        advance();
        return true;
    }
    return false;
}

void Compiler::emit(uint8_t byte) {
    code.push_back(byte);
}

void Compiler::emitInt32(int32_t value) {
    code.push_back(value & 0xFF);
    code.push_back((value >> 8) & 0xFF);
    code.push_back((value >> 16) & 0xFF);
    code.push_back((value >> 24) & 0xFF);
}

void Compiler::emitString(const char* str) {
    size_t len = strLen(str);
    if (len > 255) len = 255;
    code.push_back(static_cast<uint8_t>(len));
    for (size_t i = 0; i < len; i++) {
        code.push_back(static_cast<uint8_t>(str[i]));
    }
}

void Compiler::makeLabel(char* buf) {
    buf[0] = 'L';
    int n = labelCounter++;
    int i = 1;
    if (n == 0) { buf[i++] = '0'; }
    else {
        char digits[16];
        int j = 0;
        while (n > 0) {
            digits[j++] = '0' + (n % 10);
            n /= 10;
        }
        while (j > 0) {
            buf[i++] = digits[--j];
        }
    }
    buf[i] = '\0';
}

void Compiler::label(const char* name) {
    if (labelCount < 64) {
        strCpy(labels[labelCount].name, name, 16);
        labels[labelCount].address = code.size();
        labelCount++;
    }
}

void Compiler::emitJump(uint8_t opcode, const char* labelName) {
    emit(opcode);
    if (jumpCount < 128) {
        jumps[jumpCount].position = code.size();
        strCpy(jumps[jumpCount].label, labelName, 16);
        jumpCount++;
    }
    emitInt32(0);
}

// Public compile method
std::vector<uint8_t>& Compiler::compile() {
    while (current().type != TokenType::END_OF_FILE) {
        statement();
    }

    emit(OP_HALT);

    // Resolve jumps
    for (size_t i = 0; i < jumpCount; i++) {
        size_t jumpPos = jumps[i].position;
        const char* labelName = jumps[i].label;

        // Find label
        int32_t target = -1;
        for (size_t j = 0; j < labelCount; j++) {
            if (strEq(labels[j].name, labelName)) {
                target = labels[j].address;
                break;
            }
        }

        if (target >= 0) {
            code[jumpPos] = target & 0xFF;
            code[jumpPos + 1] = (target >> 8) & 0xFF;
            code[jumpPos + 2] = (target >> 16) & 0xFF;
            code[jumpPos + 3] = (target >> 24) & 0xFF;
        }
    }

    return code;
}

// Statement parsing methods
void Compiler::statement() {
    if (match(TokenType::VAR)) {
        varDeclaration();
    }
    else if (match(TokenType::IF)) {
        ifStatement();
    }
    else if (match(TokenType::WHILE)) {
        whileStatement();
    }
    else if (match(TokenType::PRINT)) {
        printStatement();
    }
    else if (match(TokenType::LBRACE)) {
        block();
    }
    else {
        expressionStatement();
    }
}

void Compiler::varDeclaration() {
    char name[32];
    strCpy(name, current().value, 32);
    match(TokenType::IDENTIFIER);
    
    if (match(TokenType::ASSIGN)) {
        expression();
    } else {
        emit(OP_PUSH);
        emitInt32(0);
    }
    
    emit(OP_STORE);
    emitString(name);
    match(TokenType::SEMICOLON);
}

void Compiler::ifStatement() {
    match(TokenType::LPAREN);
    expression();
    match(TokenType::RPAREN);
    
    char elseLabel[16], endLabel[16];
    makeLabel(elseLabel);
    makeLabel(endLabel);
    
    emitJump(OP_JMP_NOT, elseLabel);
    statement();
    
    if (match(TokenType::ELSE)) {
        emitJump(OP_JMP, endLabel);
        label(elseLabel);
        statement();
        label(endLabel);
    } else {
        label(elseLabel);
    }
}

void Compiler::whileStatement() {
    char startLabel[16], endLabel[16];
    makeLabel(startLabel);
    makeLabel(endLabel);
    
    label(startLabel);
    match(TokenType::LPAREN);
    expression();
    match(TokenType::RPAREN);
    
    emitJump(OP_JMP_NOT, endLabel);
    statement();
    emitJump(OP_JMP, startLabel);
    label(endLabel);
}

void Compiler::printStatement() {
    match(TokenType::LPAREN);
    expression();
    match(TokenType::RPAREN);
    emit(OP_PRINT);
    match(TokenType::SEMICOLON);
}

void Compiler::block() {
    while (current().type != TokenType::RBRACE && current().type != TokenType::END_OF_FILE) {
        statement();
    }
    match(TokenType::RBRACE);
}

void Compiler::expressionStatement() {
    expression();
    emit(OP_POP);
    match(TokenType::SEMICOLON);
}

// Expression parsing methods
void Compiler::expression() {
    if (current().type == TokenType::IDENTIFIER && peek().type == TokenType::ASSIGN) {
        char name[32];
        strCpy(name, current().value, 32);
        advance();
        advance();
        expression();
        emit(OP_STORE);
        emitString(name);
    } else {
        logicalOr();
    }
}

void Compiler::logicalOr() {
    logicalAnd();
    while (match(TokenType::OR)) {
        logicalAnd();
        emit(OP_OR);
    }
}

void Compiler::logicalAnd() {
    equality();
    while (match(TokenType::AND)) {
        equality();
        emit(OP_AND);
    }
}

void Compiler::equality() {
    comparison();
    while (true) {
        if (match(TokenType::EQ)) {
            comparison();
            emit(OP_EQ);
        } else if (match(TokenType::NE)) {
            comparison();
            emit(OP_NE);
        } else break;
    }
}

void Compiler::comparison() {
    term();
    while (true) {
        if (match(TokenType::LT)) {
            term();
            emit(OP_LT);
        } else if (match(TokenType::LE)) {
            term();
            emit(OP_LE);
        } else if (match(TokenType::GT)) {
            term();
            emit(OP_GT);
        } else if (match(TokenType::GE)) {
            term();
            emit(OP_GE);
        } else break;
    }
}

void Compiler::term() {
    factor();
    while (true) {
        if (match(TokenType::PLUS)) {
            factor();
            emit(OP_ADD);
        } else if (match(TokenType::MINUS)) {
            factor();
            emit(OP_SUB);
        } else break;
    }
}

void Compiler::factor() {
    unary();
    while (true) {
        if (match(TokenType::STAR)) {
            unary();
            emit(OP_MUL);
        } else if (match(TokenType::SLASH)) {
            unary();
            emit(OP_DIV);
        } else if (match(TokenType::PERCENT)) {
            unary();
            emit(OP_MOD);
        } else break;
    }
}

void Compiler::unary() {
    if (match(TokenType::MINUS)) {
        unary();
        emit(OP_NEG);
    } else if (match(TokenType::NOT)) {
        unary();
        emit(OP_NOT);
    } else {
        primary();
    }
}

void Compiler::primary() {
    if (match(TokenType::NUMBER)) {
        int32_t value = toInt(tokens[pos - 1].value);
        emit(OP_PUSH);
        emitInt32(value);
    }
    else if (match(TokenType::IDENTIFIER)) {
        char name[32];
        strCpy(name, tokens[pos - 1].value, 32);
        emit(OP_LOAD);
        emitString(name);
    }
    else if (match(TokenType::LPAREN)) {
        expression();
        match(TokenType::RPAREN);
    }
}
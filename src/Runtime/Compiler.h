#ifndef COMPILER_H
#define COMPILER_H

#include <vector>
#include <cstdint>
#include "Lexer.h"

class Compiler {
private:
    std::vector<Token>& tokens;
    size_t pos;
    std::vector<uint8_t> code;

    Label labels[64];
    size_t labelCount;

    Jump jumps[128];
    size_t jumpCount;

    int labelCounter;

    Token& current();
    Token& peek(int offset = 1);
    void advance();
    bool match(TokenType type);
    void emit(uint8_t byte);
    void emitInt32(int32_t value);
    void emitString(const char* str);
    void makeLabel(char* buf);
    void label(const char* name);
    void emitJump(uint8_t opcode, const char* labelName);

    void expression();
    void logicalOr();
    void logicalAnd();
    void equality();
    void comparison();
    void term();
    void factor();
    void unary();
    void primary();
    void statement();
    void varDeclaration();
    void ifStatement();
    void whileStatement();
    void printStatement();
    void block();
    void expressionStatement();

public:
    Compiler(std::vector<Token>& toks);
    std::vector<uint8_t>& compile();
};

#endif

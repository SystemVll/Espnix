#include "Lexer.h"
#include <vector>
#include <string>
#include <cstdint>

bool strEq(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

void strCpy(char* dst, const char* src, size_t max) {
    size_t i = 0;
    while (src[i] && i < max - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

size_t strLen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
bool isAlnum(char c) { return isAlpha(c) || isDigit(c); }
bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

int32_t toInt(const char* str) {
    int32_t result = 0;
    bool negative = false;
    size_t i = 0;

    if (str[0] == '-') {
        negative = true;
        i = 1;
    }

    while (str[i]) {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return negative ? -result : result;
}

Lexer::Lexer(const char* src) : source(src), pos(0), line(1) {}

char Lexer::current() {
    return source[pos];
}

char Lexer::peek(int offset) {
    return source[pos + offset];
}

void Lexer::advance() {
    if (current() == '\n') line++;
    pos++;
}

void Lexer::skipWhitespace() {
    while (isSpace(current())) advance();
}

void Lexer::skipComment() {
    if (current() == '/' && peek() == '/') {
        while (current() != '\n' && current() != '\0') advance();
    }
}

void Lexer::addToken(TokenType type, const char* val) {
    Token tok;
    tok.type = type;
    tok.line = line;
    strCpy(tok.value, val, 32);
    tokens.push_back(tok);
}

void Lexer::number() {
    char num[32];
    size_t i = 0;
    while (isDigit(current()) && i < 31) {
        num[i++] = current();
        advance();
    }
    num[i] = '\0';
    addToken(TokenType::NUMBER, num);
}

void Lexer::identifier() {
    char id[32];
    size_t i = 0;
    while ((isAlnum(current()) || current() == '_') && i < 31) {
        id[i++] = current();
        advance();
    }
    id[i] = '\0';

    if (strEq(id, "if")) addToken(TokenType::IF, id);
    else if (strEq(id, "else")) addToken(TokenType::ELSE, id);
    else if (strEq(id, "while")) addToken(TokenType::WHILE, id);
    else if (strEq(id, "var")) addToken(TokenType::VAR, id);
    else if (strEq(id, "print")) addToken(TokenType::PRINT, id);
    else if (strEq(id, "sleep")) addToken(TokenType::SLEEP, id);
    else if (strEq(id, "and")) addToken(TokenType::AND, id);
    else if (strEq(id, "or")) addToken(TokenType::OR, id);
    else if (strEq(id, "not")) addToken(TokenType::NOT, id);
    else addToken(TokenType::IDENTIFIER, id);
}

std::vector<Token>& Lexer::tokenize() {
    while (current() != '\0') {
        skipWhitespace();
        skipComment();

        if (current() == '\0') break;

        if (isDigit(current())) {
            number();
        }
        else if (isAlpha(current()) || current() == '_') {
            identifier();
        }
        else if (current() == '+') {
            addToken(TokenType::PLUS, "+");
            advance();
        }
        else if (current() == '-') {
            addToken(TokenType::MINUS, "-");
            advance();
        }
        else if (current() == '*') {
            addToken(TokenType::STAR, "*");
            advance();
        }
        else if (current() == '/') {
            addToken(TokenType::SLASH, "/");
            advance();
        }
        else if (current() == '%') {
            addToken(TokenType::PERCENT, "%");
            advance();
        }
        else if (current() == '=') {
            if (peek() == '=') {
                addToken(TokenType::EQ, "==");
                advance(); advance();
            } else {
                addToken(TokenType::ASSIGN, "=");
                advance();
            }
        }
        else if (current() == '!') {
            if (peek() == '=') {
                addToken(TokenType::NE, "!=");
                advance(); advance();
            } else {
                addToken(TokenType::NOT, "!");
                advance();
            }
        }
        else if (current() == '<') {
            if (peek() == '=') {
                addToken(TokenType::LE, "<=");
                advance(); advance();
            } else {
                addToken(TokenType::LT, "<");
                advance();
            }
        }
        else if (current() == '>') {
            if (peek() == '=') {
                addToken(TokenType::GE, ">=");
                advance(); advance();
            } else {
                addToken(TokenType::GT, ">");
                advance();
            }
        }
        else if (current() == '&' && peek() == '&') {
            addToken(TokenType::AND, "&&");
            advance(); advance();
        }
        else if (current() == '|' && peek() == '|') {
            addToken(TokenType::OR, "||");
            advance(); advance();
        }
        else if (current() == '(') {
            addToken(TokenType::LPAREN, "(");
            advance();
        }
        else if (current() == ')') {
            addToken(TokenType::RPAREN, ")");
            advance();
        }
        else if (current() == '{') {
            addToken(TokenType::LBRACE, "{");
            advance();
        }
        else if (current() == '}') {
            addToken(TokenType::RBRACE, "}");
            advance();
        }
        else if (current() == ';') {
            addToken(TokenType::SEMICOLON, ";");
            advance();
        }
        else if (current() == ',') {
            addToken(TokenType::COMMA, ",");
            advance();
        }
        else {
            advance();
        }
    }

    Token eof;
    eof.type = TokenType::END_OF_FILE;
    eof.line = line;
    eof.value[0] = '\0';
    tokens.push_back(eof);

    return tokens;
}


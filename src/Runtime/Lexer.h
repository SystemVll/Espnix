#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include <cstdint>

// Helper structs
struct Variable {
    char name[32];
    int32_t value;
};

struct Label {
    char name[16];
    size_t address;
};

struct Jump {
    size_t position;
    char label[16];
};

// Helper functions
bool strEq(const char* a, const char* b);
void strCpy(char* dst, const char* src, size_t max);
size_t strLen(const char* s);
bool isDigit(char c);
bool isAlpha(char c);
bool isAlnum(char c);
bool isSpace(char c);
int32_t toInt(const char* str);

// Token types
enum class TokenType {
    NUMBER, IDENTIFIER,
    IF, ELSE, WHILE, VAR, PRINT,
    PLUS, MINUS, STAR, SLASH, PERCENT,
    ASSIGN, EQ, NE, LT, LE, GT, GE,
    AND, OR, NOT,
    LPAREN, RPAREN, LBRACE, RBRACE,
    SEMICOLON, COMMA,
    END_OF_FILE, UNKNOWN
};

struct Token {
    TokenType type;
    char value[32];
    int line;
};

class Lexer {
private:
    const char* source;
    size_t pos;
    int line;
    std::vector<Token> tokens;

    char current();
    char peek(int offset = 1);
    void advance();
    void skipWhitespace();
    void skipComment();
    void addToken(TokenType type, const char* val);
    void number();
    void identifier();

public:
    Lexer(const char* src);
    std::vector<Token>& tokenize();
};

#endif


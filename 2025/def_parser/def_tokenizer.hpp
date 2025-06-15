#pragma once
#include <string>
#include <fstream>
#include <optional>

enum class TokenKind {
    Identifier, Number, Symbol, EndOfFile
};

struct Token {
    TokenKind kind;
    std::string text;
    int line = 0;
};

class Tokenizer {
public:
    explicit Tokenizer(std::istream& in);
    Token next();
    Token peek();

private:
    std::istream& input;
    Token buffer;
    bool hasBuffer = false;
    int line = 1;

    Token lexToken();
};

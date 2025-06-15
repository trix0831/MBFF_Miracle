#include "def_tokenizer.hpp"
#include <cctype>

Tokenizer::Tokenizer(std::istream& in) : input(in) {}

Token Tokenizer::next() {
    if (hasBuffer) {
        hasBuffer = false;
        return buffer;
    }
    return lexToken();
}

Token Tokenizer::peek() {
    if (!hasBuffer) {
        buffer = lexToken();
        hasBuffer = true;
    }
    return buffer;
}

Token Tokenizer::lexToken() {
    char ch;
    while (input.get(ch)) {
        if (ch == '"') {
            Token tok;
            tok.kind = TokenKind::Identifier;  // treat string literals like identifiers
            tok.line = line;
            tok.text = "";

            while (input.peek() && input.peek() != '"') {
                tok.text += static_cast<char>(input.get());
            }

            if (input.peek() == '"') input.get();  // consume closing quote
            return tok;
        }

        if (ch == '\n') ++line;
        if (std::isspace(ch)) continue;

        Token tok;
        tok.line = line;

        if (std::isalpha(ch) || ch == '_') {
            tok.kind = TokenKind::Identifier;
            tok.text = ch;
            while (input.peek() && (std::isalnum(input.peek()) || input.peek() == '_' || input.peek() == '/' || 
                   input.peek() == '[' || input.peek() == ']' || input.peek() == '\\'))  // Allow [, ], and \ in identifiers
                tok.text += static_cast<char>(input.get());
            return tok;
        }

        if (std::isdigit(ch) || ch == '-') {
            tok.kind = TokenKind::Number;
            tok.text = ch;
            bool seenDot = false;

            while (true) {
                char next = input.peek();
                if (std::isdigit(next)) {
                    tok.text += input.get();
                } else if (next == '.' && !seenDot) {
                    tok.text += input.get();
                    seenDot = true;
                } else {
                    break;
                }
            }

            return tok;
        }

        if (ch == ';' || ch == '-' || ch == '+' || ch == '(' || ch == ')') {
            tok.kind = TokenKind::Symbol;
            tok.text = ch;
            return tok;
        }
    }

    return Token{TokenKind::EndOfFile, "", line};
}

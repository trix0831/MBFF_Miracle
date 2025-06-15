#include "lef_tokenizer.hpp"
#include <cctype>

namespace lef {

/*---------------------------------------------------------------------------*/
Token Tokenizer::peek()
{
    if (!hasBuf_) { buf_ = lex(); hasBuf_ = true; }
    return buf_;
}

Token Tokenizer::next()
{
    if (hasBuf_) { hasBuf_ = false; return buf_; }
    return lex();
}

/*---------------------------------------------------------------------------*/
static bool isIdentStart(int c) { return std::isalpha(c) || c == '_' || c == '$'; }
static bool isIdentBody (int c)
{
    return std::isalnum(c) || c == '_' || c == '$' || c == '/' || c == '[' || c == ']' || c == '.';
}

Token Tokenizer::lex()
{
    char ch;
    while (in_.get(ch)) {
        /* -------- whitespace & comments (# … EOL) ------------------------- */
        if (ch == '\n') { ++line_; continue; }
        if (std::isspace(static_cast<unsigned char>(ch))) continue;

        if (ch == '#') {                             // comment
            while (in_.peek() != '\n' && in_) in_.get();
            continue;
        }

        Token tok; tok.line = line_;

        /* -------- string literal ----------------------------------------- */
        if (ch == '"') {
            tok.kind = TokenKind::String;
            while (in_.peek() != '"' && in_) tok.text += static_cast<char>(in_.get());
            if (in_.peek() == '"') in_.get();        // consume closing quote
            return tok;
        }

        /* -------- identifier --------------------------------------------- */
        if (isIdentStart(ch)) {
            tok.kind = TokenKind::Identifier;
            tok.text += ch;
            while (isIdentBody(in_.peek())) tok.text += static_cast<char>(in_.get());
            return tok;
        }

        /* -------- number ------------------------------------------------- */
        if (std::isdigit(ch) || (ch == '-' && std::isdigit(in_.peek()))) {
            tok.kind = TokenKind::Number;
            tok.text += ch;
            bool hasDot = false, hasExp = false;
            while (true) {
                int p = in_.peek();
                if (std::isdigit(p)) tok.text += static_cast<char>(in_.get());
                else if (p == '.' && !hasDot) { hasDot = true; tok.text += static_cast<char>(in_.get()); }
                else if ((p == 'e' || p == 'E') && !hasExp) {
                    hasExp = true; tok.text += static_cast<char>(in_.get());
                    if (in_.peek() == '+' || in_.peek() == '-') tok.text += static_cast<char>(in_.get());
                }
                else break;
            }
            return tok;
        }

        /* -------- single-char symbol ------------------------------------- */
        if (ch == '(' || ch == ')' || ch == ';' || ch == '+' || ch == '-' || ch == ',') {
            tok.kind = TokenKind::Symbol;
            tok.text = ch;
            return tok;
        }
    }
    return Token{TokenKind::EndOfFile, "", line_};
}

} // namespace lef

#include "sdc_tokenizer.hpp"
#include <cctype>
#include <limits>

using namespace sdc;

Tokenizer::Tokenizer(std::istream& in) : input_(in) {
    cur_ = lexToken();
}

//–– helpers ------------------------------------------------------------------
static bool isIdentStart(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c=='_'||c=='-'; }
static bool isIdentChar (char c) { return isIdentStart(c) || std::isdigit(static_cast<unsigned char>(c)) || c=='/'||c=='['||c==']' || c=='{'||c=='}'; }

void Tokenizer::skipWSandComments() {
    while (true) {
        int c = input_.peek();
        if (c==EOF) return;
        if (std::isspace(c)) { input_.get(); continue; }

        /* comment start?  (# …)  or  // …                                    */
        if (c=='#') {                    // TCL-style comment
            input_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        if (c=='/') {
            input_.get();
            if (input_.peek()=='/') {
                input_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            input_.putback('/');
            return;
        }
        return;                         // non-whitespace / non-comment char
    }
}

Token Tokenizer::lexToken() {
    skipWSandComments();
    int c = input_.peek();
    if (c==EOF) return {TokenKind::EndOfFile, ""};

    // String starting with { … } or [ … ]
    if (c=='{' || c=='[') {
        char open = static_cast<char>(input_.get());
        char close = (open=='{') ? '}' : ']';
        std::string s(1, open);
        int nest = 1;
        while (nest>0 && input_.good()) {
            char ch = static_cast<char>(input_.get());
            s += ch;
            if (ch==open)  ++nest;
            if (ch==close) --nest;
        }
        return {TokenKind::String, s};
    }

    // Number
    if (std::isdigit(c) || c=='.') {
        std::string num;
        while (input_.good()) {
            c = input_.peek();
            if (std::isdigit(c) || c=='.' || c=='e' || c=='E' || c=='+'||c=='-')
                { num += static_cast<char>(input_.get()); }
            else break;
        }
        return {TokenKind::Number, num};
    }

    // Identifier / option flag
    if (isIdentStart(c)) {
        std::string id;
        while (input_.good()) {
            c = input_.peek();
            if (isIdentChar(c))
                { id += static_cast<char>(input_.get()); }
            else break;
        }
        return {TokenKind::Identifier, id};
    }

    // unknown single char – treat as identifier so parser can skip
    input_.get();
    return {TokenKind::Unknown, std::string(1, static_cast<char>(c))};
}

Token Tokenizer::next() {
    Token ret = cur_;
    cur_ = lexToken();
    return ret;
}

#pragma once
/*---------------------------------------------------------------------------*
 *  Tiny tokenizer for LEF                                                                 *
 *---------------------------------------------------------------------------*/
#include <istream>
#include <string>

namespace lef {

enum class TokenKind { Identifier, Number, String, Symbol, EndOfFile };

struct Token {
    TokenKind   kind{TokenKind::EndOfFile};
    std::string text;
    int         line{0};
};

class Tokenizer {
public:
    explicit Tokenizer(std::istream& in) : in_(in) {}
    Token next();        ///< consume & return next token
    Token peek();        ///< look-ahead (no consume)

private:
    std::istream& in_;
    Token         buf_;
    bool          hasBuf_{false};
    int           line_{1};

    Token lex();         ///< actual scanner
};

} // namespace lef

#ifndef SDC_TOKENIZER_HPP
#define SDC_TOKENIZER_HPP
#include <istream>
#include <string>

namespace sdc {

enum class TokenKind { Identifier, Number, String, EndOfFile, Unknown };

struct Token {
    TokenKind   kind   = TokenKind::Unknown;
    std::string lexeme;
};

class Tokenizer {
public:
    explicit Tokenizer(std::istream& in);

    /// Return **current** token without consuming it
    const Token& peek() const { return cur_; }

    /// Consume current token and return it, then advance
    Token next();

private:
    std::istream& input_;
    Token         cur_;

    void  skipWSandComments();
    Token lexToken();
};

} // namespace sdc
#endif /* SDC_TOKENIZER_HPP */

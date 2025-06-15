#ifndef SDC_PARSER_HPP
#define SDC_PARSER_HPP
#include "sdc_design.hpp"
#include "sdc_tokenizer.hpp"
#include <stdexcept>

namespace sdc {

class SdcParser {
public:
    explicit SdcParser(std::istream& in) : lexer_(in) {}
    Sdc parse();

private:
    Tokenizer lexer_;

    // helpers ---------------------------------------------------------------
    const Token& peek() const { return lexer_.peek(); }
    Token        next()       { return lexer_.next(); }

    Token expect(TokenKind k, const std::string& msg);


    // command parsers -------------------------------------------------------
    void parseSet      (Sdc& db);
    void parseSetUnits (Sdc& db);
    void parseLimits   (Sdc& db);
    void parseCreateClock      (Sdc& db);
    void parseSetLoad          (Sdc& db);
    void parseClockValueCmd    (Sdc& db, const std::string& cmd);
    void parseDelay            (Sdc& db, bool isInput);
};

} // namespace sdc
#endif /* SDC_PARSER_HPP */

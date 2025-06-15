#pragma once
/*---------------------------------------------------------------------------*
 *  Recursive-descent LEF parser                                            *
 *---------------------------------------------------------------------------*/
#include "lef_design.hpp"
#include "lef_tokenizer.hpp"
#include <stdexcept>

namespace lef {

class LefParser {
public:
    explicit LefParser(std::istream& in) : tz_(in) {}
    Library parse();                         ///< entry point

private:
    /* core ---------------------------------------------------------------- */
    Tokenizer tz_;
    Library   lib_;
    uint32_t  nextMacroId_{0};

    /* generic helpers ----------------------------------------------------- */
    Token expect(TokenKind k);
    Token expect(const std::string& txt);    ///< expect exact identifier / symbol
    bool  accept(const std::string& txt);    ///< if next matches, consume & true
    void  skipToEnd(const std::string& kw);  ///< skip until END kw ;
    double toMicron(const std::string& n) const;

    /* section dispatch ---------------------------------------------------- */
    void parseHeader();                      // UNITS, MANUFACTURINGGRID
    void parseSite();                        // SITE … END SITE
    void parseMacro();                       // MACRO … END MACRO

    /* nested inside MACRO ------------------------------------------------- */
    void parsePin  (Macro& mc);
    void parsePort (Pin& p);
    void parseObs  (Macro& mc);
};

} // namespace lef

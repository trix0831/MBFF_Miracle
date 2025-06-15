#pragma once
#include "design.hpp"
#include "def_tokenizer.hpp"

class DefParser {
public:
    explicit DefParser(std::istream& in);
    Design parse();

private:
    Tokenizer tokenizer;

    Token expect(TokenKind expected);
    void parseHeader(Design& design);
    void parseDieArea(Design& design);
    void parseRows(Design& design);
    void parseTracks(Design& design);
    void parseComponents(Design& design);
    void parsePins(Design& design);
    void parsePinProperties(Design& design);
    void parseNets(Design& design);
    Orient parseOrient(const std::string& str);
    Status parseStatus(const std::string& str);
    Direction parseDirection(const std::string& str);
};

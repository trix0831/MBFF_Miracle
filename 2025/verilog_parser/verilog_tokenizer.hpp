#pragma once
#include <istream>
#include <string>
#include <unordered_map>

namespace verilog {

enum class TK {
    Identifier, Number,
    Module, Endmodule,
    Input, Output, Inout, Wire, Assign,      // ← new keyword
    Dot, Comma, Semi, LPar, RPar, LBrack, RBrack, Colon,
    Equal,                                   // ← '='
    EndOfFile
};

struct Token { TK kind = TK::EndOfFile; std::string lexeme; int line = 1; };

class VerilogTokenizer {
public:
    explicit VerilogTokenizer(std::istream& in):in_(in){ initKeywords(); next(); }
    const Token& peek() const { return cur_; }
    Token next();

private:
    std::istream& in_;  Token cur_;  int c_=' ';  std::unordered_map<std::string,TK> kw_;

    void initKeywords();
    void advance(){ c_ = in_.get(); if(c_=='\n') ++cur_.line; }
    void skipWSandComments();
    Token makeIdentifier();
    Token makeEscapedIdentifier();
    Token makeNumber();
};

} // namespace verilog

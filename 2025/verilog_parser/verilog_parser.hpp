#pragma once
#include "verilog_design.hpp"
#include "verilog_tokenizer.hpp"
#include <istream>

namespace verilog {

class VerilogParser {
public:
    explicit VerilogParser(std::istream& in): lexer(in) {}
    VerilogDesign parse();

private:
    VerilogTokenizer lexer;
    const Token& tok() const { return lexer.peek(); }
    Token expect(TK,const char*);
    bool  accept(TK);

    /* grammar pieces */
    void parseModule      (VerilogDesign&);
    void parsePortList    (VerilogModule&);
    void parsePortDirDecl (VerilogModule&,PortDir);
    void parseWireDecl    (VerilogModule&);
    void parseInstance    (VerilogModule&);
    void parseAssign      (VerilogModule&);      // ← new

    /* helpers */
    std::string parseNetExpr();
    void parseRange();
    void addNetIfMissing(VerilogModule&,const std::string&);
    static PortDir kw2dir(TK);
};

} // namespace verilog

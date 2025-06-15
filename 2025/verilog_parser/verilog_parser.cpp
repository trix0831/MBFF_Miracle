#include "verilog_parser.hpp"
#include <sstream>
#include <stdexcept>
# include <iostream>
namespace verilog {

/*----- tiny utils ----------------------------------------------------------*/
Token VerilogParser::expect(TK k,const char* msg){
    if(tok().kind!=k){
        std::ostringstream oss;
        oss << "Parse error: expected " << msg;
        throw std::runtime_error(oss.str());
    }
    Token t = tok();      // Keep the token we just verified
    lexer.next();         // Then advance to the next one
    return t;             // Return the correct token
}

bool VerilogParser::accept(TK k){ if(tok().kind==k){ lexer.next(); return true;} return false;}

PortDir VerilogParser::kw2dir(TK k){
    if(k==TK::Input)  return PortDir::INPUT;
    if(k==TK::Output) return PortDir::OUTPUT;
    return PortDir::INOUT;
}
void VerilogParser::addNetIfMissing(VerilogModule& m,const std::string& n){
    for(auto& e:m.nets) if(e.name==n) return;
    m.nets.push_back({n});
}

/*----- range helpers -------------------------------------------------------*/
void VerilogParser::parseRange(){
    expect(TK::LBrack,"'['");
    expect(TK::Number,"number");
    if(accept(TK::Colon)) expect(TK::Number,"number");
    expect(TK::RBrack,"']'");
}

/* netExpr -> Identifier ('[' … ']')* --------------------------------------*/
std::string VerilogParser::parseNetExpr(){
    std::string out = expect(TK::Identifier,"net").lexeme;
    while(tok().kind==TK::LBrack){
        out.push_back('['); lexer.next();
        out += expect(TK::Number,"index").lexeme;
        if(accept(TK::Colon)){ out.push_back(':'); out+=expect(TK::Number,"index").lexeme; }
        expect(TK::RBrack,"]"); out.push_back(']');
    }
    return out;
}

/*---------------------------------------------------------------------------*/
VerilogDesign VerilogParser::parse(){
    VerilogDesign d;
    while(tok().kind!=TK::EndOfFile) parseModule(d);
    return d;
}

/*----- module --------------------------------------------------------------*/
void VerilogParser::parseModule(VerilogDesign& d){
    expect(TK::Module,"module");
    d.mod.name = expect(TK::Identifier,"module name").lexeme;

    if(accept(TK::LPar)){ parsePortList(d.mod); expect(TK::RPar,"')'"); }
    expect(TK::Semi,"';'");

    while(tok().kind!=TK::Endmodule){
        if(tok().kind==TK::Input||tok().kind==TK::Output||tok().kind==TK::Inout)
            parsePortDirDecl(d.mod,kw2dir(tok().kind));
        else if(tok().kind==TK::Wire)
            parseWireDecl(d.mod);
        else if(tok().kind==TK::Assign)
            parseAssign(d.mod);
        else if(tok().kind==TK::Identifier)
            parseInstance(d.mod);
        else
            throw std::runtime_error("Unexpected token inside module body");
    }
    expect(TK::Endmodule,"endmodule");
}

/*----- header port list ----------------------------------------------------*/
void VerilogParser::parsePortList(VerilogModule& m){
    bool first = true;
    for(;;){
        if(tok().kind==TK::RPar) break;
        if(!first) expect(TK::Comma,"','");
        first=false;
        std::string portName;
        if(tok().kind==TK::Identifier) {
            portName = tok().lexeme;
            lexer.next();
        } else {
            throw std::runtime_error("Expected port identifier");
        }
        m.ports.push_back({portName,PortDir::INPUT});
    }
}

/*----- direction-specific declarations ------------------------------------*/
void VerilogParser::parsePortDirDecl(VerilogModule& m,PortDir dir){
    lexer.next();                      // consume keyword
    if(tok().kind==TK::LBrack) parseRange();           // shared range
    bool first=true;
    for(;;){
        if(!first) expect(TK::Comma,"','"); first=false;
        if(tok().kind==TK::LBrack) parseRange();       // per-signal range
        auto id = expect(TK::Identifier,"identifier").lexeme;
        bool seen=false;
        for(auto& p:m.ports) if(p.name==id){ p.dir=dir; seen=true; break; }
        if(!seen) m.ports.push_back({id,dir});
        if(!accept(TK::Comma)) break;
    }
    expect(TK::Semi,"';'");
}

/*----- wire declarations ---------------------------------------------------*/
void VerilogParser::parseWireDecl(VerilogModule& m){
    lexer.next(); if(tok().kind==TK::LBrack) parseRange();
    bool first=true;
    for(;;){
        if(!first) expect(TK::Comma,"','"); first=false;
        if(tok().kind==TK::LBrack) parseRange();
        std::string id;
        if(tok().kind==TK::Identifier) {
            id = tok().lexeme;
            lexer.next();
        } else {
            throw std::runtime_error("Expected wire identifier");
        }
        addNetIfMissing(m,id);
        if(!accept(TK::Comma)) break;
    }
    expect(TK::Semi,"';'");
}

/*----- assign --------------------------------------------------------------*/
void VerilogParser::parseAssign(VerilogModule& m){
    expect(TK::Assign,"assign");
    auto lhs = parseNetExpr();
    expect(TK::Equal,"'='");
    auto rhs = parseNetExpr();
    expect(TK::Semi,"';'");

    auto baseL = lhs.substr(0,lhs.find('['));
    auto baseR = rhs.substr(0,rhs.find('['));
    addNetIfMissing(m,baseL);
    addNetIfMissing(m,baseR);
}

/*----- instance ------------------------------------------------------------*/
void VerilogParser::parseInstance(VerilogModule& m){
    VerilogInstance inst;
    inst.cellType = expect(TK::Identifier,"cell type").lexeme;
    inst.instName = expect(TK::Identifier,"instance name").lexeme;
    expect(TK::LPar,"'('");
    bool first=true;
    while(!accept(TK::RPar)){
        if(!first) expect(TK::Comma,"','"); first=false;
        expect(TK::Dot,"'.'");
        auto pin = expect(TK::Identifier,"pin").lexeme;
        expect(TK::LPar,"'('");
        auto netExpr = parseNetExpr();
        expect(TK::RPar,"')'");
        inst.pin2net[pin]=netExpr;
        std::string base = netExpr.substr(0,netExpr.find('['));
        if(base.empty()) base = netExpr; // Handle case where there's no array indexing
        addNetIfMissing(m,base);
    }
    expect(TK::Semi,"';'");
    m.instances.push_back(inst);
}

} // namespace verilog

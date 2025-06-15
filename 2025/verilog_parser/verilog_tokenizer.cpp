#include "verilog_tokenizer.hpp"
#include <cctype>
#include <stdexcept>

namespace verilog {

/*-------------------------------------------------------------------------*/
void VerilogTokenizer::initKeywords(){
    kw_ = { {"module",TK::Module}, {"endmodule",TK::Endmodule},
            {"input",TK::Input},   {"output",TK::Output},
            {"inout",TK::Inout},   {"wire",TK::Wire},
            {"assign",TK::Assign} };
}

/*-------------------------------------------------------------------------*/
void VerilogTokenizer::skipWSandComments(){
    for(;;){
        while(isspace(c_)) advance();
        if(c_=='/' && in_.peek()=='/'){           // // comment
            while(c_!='\n' && c_!=EOF) advance();
            continue;
        }
        if(c_=='/' && in_.peek()=='*'){           /* block comment */
            advance(); advance();
            while(!(c_=='*' && in_.peek()=='/')){
                if(c_==EOF) throw std::runtime_error("Unterminated comment");
                advance();
            }
            advance(); advance();
            continue;
        }
        break;
    }
}

/*-------------------------------------------------------------------------*/
Token VerilogTokenizer::makeNumber(){
    Token t; t.kind=TK::Number;
    while(isdigit(c_)){ t.lexeme.push_back(c_); advance(); }
    return t;
}

/* identifier or keyword --------------------------------------------------*/
Token VerilogTokenizer::makeIdentifier(){
    Token t; t.kind=TK::Identifier;
    while(isalnum(c_)||c_=='_'||c_=='$'||c_=='.') { 
        t.lexeme.push_back(c_); 
        advance(); 
    }
    auto it = kw_.find(t.lexeme);
    if(it!=kw_.end()) t.kind = it->second;
    return t;
}

/* \escaped identifier ----------------------------------------------------*/
Token VerilogTokenizer::makeEscapedIdentifier(){
    Token t; t.kind=TK::Identifier;
    t.lexeme.push_back('\\'); advance();            // keep the backslash
    while(c_!=EOF && !isspace(c_) &&
          c_!='[' && c_!=']' && c_!='(' && c_!=')' &&
          c_!='.' && c_!=';' && c_!=',' && c_!=':'){
        t.lexeme.push_back(c_); advance();
    }
    // Handle array indexing for escaped identifiers
    if(c_=='[') {
        t.lexeme.push_back('[');
        advance();
        while(isdigit(c_)) {
            t.lexeme.push_back(c_);
            advance();
        }
        if(c_==':') {
            t.lexeme.push_back(':');
            advance();
            while(isdigit(c_)) {
                t.lexeme.push_back(c_);
                advance();
            }
        }
        if(c_==']') {
            t.lexeme.push_back(']');
            advance();
        }
    }
    return t;
}

/*-------------------------------------------------------------------------*/
Token VerilogTokenizer::next(){
    skipWSandComments();

    Token t; t.lexeme.clear(); t.line = cur_.line;

    switch(c_){
        case '.': t.kind=TK::Dot;    t.lexeme="."; advance(); break;
        case ',': t.kind=TK::Comma;  t.lexeme=","; advance(); break;
        case ';': t.kind=TK::Semi;   t.lexeme=";"; advance(); break;
        case '(': t.kind=TK::LPar;   t.lexeme="("; advance(); break;
        case ')': t.kind=TK::RPar;   t.lexeme=")"; advance(); break;
        case '[': t.kind=TK::LBrack; t.lexeme="["; advance(); break;
        case ']': t.kind=TK::RBrack; t.lexeme="]"; advance(); break;
        case ':': t.kind=TK::Colon;  t.lexeme=":"; advance(); break;
        case '=': t.kind=TK::Equal;  t.lexeme="="; advance(); break;
        case '\\': return cur_ = makeEscapedIdentifier();
        case EOF: t.kind=TK::EndOfFile; break;
        default:
            if(isdigit(c_))   return cur_ = makeNumber();
            if(isalpha(c_)||c_=='_') return cur_ = makeIdentifier();
            throw std::runtime_error("Tokenizer: unknown character");
    }
    return cur_ = t;
}

} // namespace verilog

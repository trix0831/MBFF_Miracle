#include "sdc_parser.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <regex>

std::string extractPortName(const std::string& expr) {
    std::regex re(R"(\[\s*get_ports\s*\{\s*([^}]+)\s*\}\s*\])");
    std::smatch m;
    if (std::regex_match(expr, m, re))
        return m[1];
    return expr; // fallback if pattern doesn't match
}

std::string extractClockName(const std::string& expr) {
    std::regex re(R"(\[\s*get_clocks\s*\{\s*([^}]+)\s*\}\s*\])");
    std::smatch m;
    if (std::regex_match(expr, m, re))
        return m[1];
    return expr; // fallback if pattern doesn't match
}

using namespace sdc;

//------------------------------------------------------------------------------
// small helpers
static double toF(const std::string& s) { return std::strtod(s.c_str(), nullptr); }

Token SdcParser::expect(TokenKind k, const std::string& msg) {
    // std::cout << "Expecting " << msg << " got '" << peek().lexeme << "'\n";
    if (peek().kind != k)
        throw std::runtime_error("SDC parse error: " + msg + " near '" + peek().lexeme + "'");
    return Token{};
}

//------------------------------------------------------------------------------
// Top-level
Sdc SdcParser::parse() {
    Sdc db;

    while (peek().kind != TokenKind::EndOfFile) {
        if (peek().kind != TokenKind::Identifier) { next(); continue; }
        std::string cmd = peek().lexeme;

        if (cmd == "set")             { parseSet(db); }
        else if (cmd == "set_units")  { parseSetUnits(db); }
        else if (cmd == "create_clock"){ parseCreateClock(db); }
        else if (cmd == "set_load")   { parseSetLoad(db); }
        else if (cmd == "set_clock_latency"
              || cmd == "set_clock_uncertainty"
              || cmd == "set_clock_transition")  { parseClockValueCmd(db, cmd); }
        else if (cmd == "set_input_delay")  { parseDelay(db, true); }
        else if (cmd == "set_output_delay") { parseDelay(db, false); }
        else if (cmd == "set_max_transition"){ parseLimits(db); }
        else if (cmd == "set_max_capacitance"){ parseLimits(db); }
        else { next(); } // skip unknown command
    }
    return db;
}

//------------------------------------------------------------------------------
// 'set' – currently only captures   set sdc_version X.Y
void SdcParser::parseSet(Sdc& db) {
    // std::cout << "Parsing 'set' command\n";
    expect(TokenKind::Identifier, "expected 'set'"); next();     // consume set
    expect(TokenKind::Identifier, "expected variable name"); 
    // std::cout << "Found 'set' variable: " << lexer_.peek().lexeme << '\n';
    std::string var = lexer_.peek().lexeme; next(); // consume variable name

    // only sdc_version used in contest
    if (var == "sdc_version") {
        // expect(TokenKind::Number, "expected version number");
        std::cout << "Setting SDC version to " << lexer_.peek().lexeme << '\n';
        db.version = lexer_.peek().lexeme; next();
    } else {                                                   // skip rest of line
        while (peek().kind!=TokenKind::EndOfFile
            && peek().kind!=TokenKind::Identifier) next();
    }
}

//------------------------------------------------------------------------------
// set_units -time ns -resistance kOhm ...
void SdcParser::parseSetUnits(Sdc& db) {
    expect(TokenKind::Identifier, "expected 'set_units'"); next(); // consume 'set_units'
    while (peek().kind==TokenKind::Identifier && peek().lexeme[0]=='-') {
        std::string opt = peek().lexeme;
        expect(TokenKind::Identifier, "unit after "+opt);next(); // consume option
        std::string unit = peek().lexeme; next(); // consume unit
        // std::cout << "Setting unit for " << opt << " to " << unit << '\n';

        if (opt=="-time")        db.timeUnit        = unit;
        else if (opt=="-resistance") db.resistanceUnit = unit;
        else if (opt=="-capacitance")db.capacitanceUnit= unit;
        else if (opt=="-voltage")    db.voltageUnit    = unit;
        else if (opt=="-current")    db.currentUnit    = unit;
    }
}

//------------------------------------------------------------------------------
// set_max_transition / set_max_capacitance
void SdcParser::parseLimits(Sdc& db) {
    std::string cmd = peek().lexeme; next();            // consume command
    expect(TokenKind::Number, "value for "+cmd);
    double val = toF(peek().lexeme); next(); // consume value
    expect(TokenKind::String, "target expr");
    std::string target = peek().lexeme; next();

    SdcLimits lim;
    lim.limitExpr = cmd; // either "set_max_transition" or "set_max_capacitance"
    lim.val = val;
    lim.targetExpr = target; // extract port name from [get_ports {...}]
    //push back
    db.limits.push_back(lim);
}

//------------------------------------------------------------------------------
// create_clock -name clk -period 1.0 -waveform {0 0.5} [get_ports {...}]
void SdcParser::parseCreateClock(Sdc& db) {
    expect(TokenKind::Identifier,"'create_clock'"); next(); // consume 'create_clock'
    CreateClock clk;

    while (peek().kind==TokenKind::Identifier && peek().lexeme[0]=='-') {
        std::string opt = next().lexeme;
        // std::cout << "Found clock option: " << opt << '\n';

        if (opt == "-name") {
            clk.name = expect(TokenKind::Identifier, "clock name").lexeme; next();
        } else if (opt == "-period") {
            // std::cout << "Setting clock period to " << peek().lexeme << '\n';
            clk.period = toF(peek().lexeme); next();
        } else if (opt == "-waveform") {
            std::string wav = peek().lexeme; next();
            double r=0.0, f=0.0;
            sscanf(wav.c_str(), "{%lf %lf}", &r, &f);
            // std::cout << "Setting clock waveform to {" << r << ", " << f << "}\n";
            clk.waveform = {r, f};
        } else {
            
        }
    }

    // std::cout << "portEpxr: " << peek().lexeme << '\n';
    clk.portExpr = extractPortName(peek().lexeme);
    // std::cout << "Clock port expression: " << clk.portExpr << '\n';

    if (clk.name.empty()) {
        clk.name = "clk" + std::to_string(db.clocks.size());
    }

    db.clocks.push_back(clk);
}

//------------------------------------------------------------------------------
// set_load -pin_load 0.1 [get_ports {...}]
void SdcParser::parseSetLoad(Sdc& db) {
    expect(TokenKind::Identifier,"'set_load'"); next(); // consume 'set_load'
    SetLoad ld;
    expect(TokenKind::Identifier,"-pin_load"); next(); // consume '-pin_load'
    expect(TokenKind::Number,"load value");
    // std::cout << "Setting pin load to " << peek().lexeme << '\n';
    ld.pinLoad = toF(peek().lexeme); next();
    expect(TokenKind::String,"port expression");
    // std::cout << "Setting port name: " << peek().lexeme << '\n';
    ld.portExpr = extractPortName(peek().lexeme); next();
    db.loads.push_back(ld);
}

//------------------------------------------------------------------------------
// set_clock_latency / uncertainty / transition  <num>  [get_clocks {...}]
void SdcParser::parseClockValueCmd(Sdc& db,const std::string& cmd) {
    next(); // consume cmd
    expect(TokenKind::Number,"value for "+cmd);
    double val = toF(peek().lexeme); next(); // consume value
    expect(TokenKind::String,"clock expression");
    std::string expr = extractClockName(peek().lexeme); next();
    // std::cout << "Setting " << cmd << " to " << val << " on " << expr << '\n';

    if (cmd=="set_clock_latency")       db.latencies     .push_back({val,expr});
    else if (cmd=="set_clock_uncertainty") db.uncertainties.push_back({val,expr});
    else if (cmd=="set_clock_transition")  db.transitions .push_back({val,expr});
}

//------------------------------------------------------------------------------
// set_input_delay / set_output_delay
void SdcParser::parseDelay(Sdc& db,bool isInput) {
    next(); // command
    expect(TokenKind::Identifier,"-clock"); next(); // -clock
    expect(TokenKind::String,"clock expr");
    std::string clkExpr = extractClockName(peek().lexeme); next();
    expect(TokenKind::Number,"delay");
    double val = toF(peek().lexeme); next();
    expect(TokenKind::String,"port expr");
    std::string portExpr = extractPortName(peek().lexeme); next();

    if (isInput) db.inputDelays .push_back({val,clkExpr,portExpr});
    else         db.outputDelays.push_back({val,clkExpr,portExpr});
}

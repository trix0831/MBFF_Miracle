#include "lef_parser.hpp"
#include <iostream>
#include <sstream>

namespace lef {

/* ──────────────────── low-level helpers ──────────────────────────────── */
Token LefParser::expect(TokenKind k)
{
    Token t = tz_.next();
    if (t.kind != k)
        throw std::runtime_error("Line " + std::to_string(t.line) +
                                 ": unexpected token '" + t.text + '\'');
    return t;
}

Token LefParser::expect(const std::string& s)
{
    Token t = tz_.next();
    if (t.text != s)
        throw std::runtime_error("Line " + std::to_string(t.line) +
                                 ": expected '" + s + "', got '" + t.text + '\'');
    return t;
}

bool LefParser::accept(const std::string& s)
{
    if (tz_.peek().text == s) { tz_.next(); return true; }
    return false;
}

void LefParser::skipToEnd(const std::string& kw)
{
    while (tz_.peek().kind != TokenKind::EndOfFile) {
        if (accept("END")) { expect(kw); return; }
        tz_.next();
    }
    throw std::runtime_error("Unterminated " + kw + " section");
}

double LefParser::toMicron(const std::string& n) const
{
    return std::stod(n);        // keep raw value; dbu→µm conversion optional
}

/* ──────────────────── top-level parse() ──────────────────────────────── */
Library LefParser::parse()
{
    parseHeader();

    while (tz_.peek().kind != TokenKind::EndOfFile) {

        /* ---------- graceful file terminator ----------------------------- */
        if (accept("END")) {                     // could be END LIBRARY
            if (accept("LIBRARY")) break;        // done!
            // else treat as unknown section label and skip
            std::string bogus = tz_.next().text;
            std::cerr << "Warning: skipping unknown END " << bogus << " block\n";
            continue;
        }

        /* ---------- normal top-level sections --------------------------- */
        if (accept("SITE"))        parseSite();
        else if (accept("MACRO"))  parseMacro();
        else {                     // unknown -> skip whole section
            std::string kw = tz_.peek().text;
            std::cerr << "Warning: skipping unknown top-level section '" << kw << "'\n";
            skipToEnd(kw);
        }
    }
    return lib_;
}

/* ──────────────────── header: VERSION / UNITS / GRID … ──────────────── */
void LefParser::parseHeader()
{
    while (true) {
        const std::string kw = tz_.peek().text;
        if (kw == "SITE" || kw == "MACRO" || kw.empty()) break;

        if (accept("VERSION")) {                  // VERSION 5.8 ;
            tz_.next();                           // number
            expect(";");                          // ;
        }
        else if (accept("BUSBITCHARS") || accept("DIVIDERCHAR")) {
            tz_.next();                           // delimiter or "[]"
            expect(";");                          // ;
        }
        else if (accept("UNITS")) {               // UNITS … END UNITS
            expect("DATABASE"); expect("MICRONS");
            lib_.units.dbuPerMicron = std::stoi(expect(TokenKind::Number).text);
            expect(";");                          // ;
            expect("END");  expect("UNITS");
        }
        else if (accept("MANUFACTURINGGRID")) {
            lib_.mfgGrid.grid = toMicron(expect(TokenKind::Number).text);
            expect(";");                          // ;
        }
        else {                                   // skip other keywords gracefully
            std::cerr << "Warning: skipping unknown header keyword '" << kw << "'\n";
            tz_.next();
        }
    }
}

/* ──────────────────── SITE block ─────────────────────────────────────── */
void LefParser::parseSite()
{
    std::string siteName = expect(TokenKind::Identifier).text;
    Site s; s.name = siteName;

    while (true) {
        if (accept("CLASS")) {                   // CLASS CORE [ANTENNACELL] … ;
            while (tz_.peek().kind == TokenKind::Identifier)
                tz_.next();
            expect(";");                         // ;
        }
        else if (accept("SIZE")) {
            s.sizeX = toMicron(expect(TokenKind::Number).text);
            expect("BY");
            s.sizeY = toMicron(expect(TokenKind::Number).text);
            expect(";");                         // ;
        }
        else if (accept("SYMMETRY")) {
            s.symmetry.clear();
            s.symmetry += expect(TokenKind::Identifier).text;
            while (tz_.peek().kind == TokenKind::Identifier) {
                s.symmetry += ' ';
                s.symmetry += tz_.next().text;
            }
            expect(";");                         // ;
        }
        else if (accept("END")) {
            expect(siteName);
            break;
        }
        else { tz_.next(); }
    }
    lib_.sites.try_emplace(siteName, std::move(s));
}

/* ──────────────────── MACRO block ────────────────────────────────────── */
void LefParser::parseMacro()
{
    std::string macroName = expect(TokenKind::Identifier).text;

    Macro mc; mc.id = nextMacroId_++; mc.name = macroName;

    while (true) {
        if (accept("CLASS")) {                   // CLASS CORE ANTENNACELL ;
            mc.cls.clear();
            mc.cls += expect(TokenKind::Identifier).text;
            while (tz_.peek().kind == TokenKind::Identifier) {
                mc.cls += ' ';
                mc.cls += tz_.next().text;
            }
            expect(";");                         // ;
        }
        else if (accept("ORIGIN")) {
            mc.origin.x = toMicron(expect(TokenKind::Number).text);
            mc.origin.y = toMicron(expect(TokenKind::Number).text);
            expect(";");                         // ;
        }
        else if (accept("SIZE")) {
            mc.sizeX = toMicron(expect(TokenKind::Number).text);
            expect("BY");
            mc.sizeY = toMicron(expect(TokenKind::Number).text);
            expect(";");                         // ;
        }
        else if (accept("SYMMETRY")) {
            mc.symmetry.clear();
            mc.symmetry += expect(TokenKind::Identifier).text;
            while (tz_.peek().kind == TokenKind::Identifier) {
                mc.symmetry += ' ';
                mc.symmetry += tz_.next().text;
            }
            expect(";");                         // ;
        }
        else if (accept("SITE")) {
            mc.site = expect(TokenKind::Identifier).text;
            expect(";");                         // ;
        }
        else if (accept("PIN")) {
            parsePin(mc);
        }
        else if (accept("OBS")) {
            parseObs(mc);
        }
        else if (accept("END")) {
            expect(macroName);
            break;
        }
        else { tz_.next(); }
    }
    lib_.macros.try_emplace(macroName, std::move(mc));
}

/* ──────────────────── PIN inside MACRO ───────────────────────────────── */
void LefParser::parsePin(Macro& mc)
{
    std::string pinName = expect(TokenKind::Identifier).text;
    Pin p; p.name = pinName;

    while (true) {
        if (accept("DIRECTION")) {
            p.direction = expect(TokenKind::Identifier).text;
            expect(";");                         // ;
        }
        else if (accept("USE")) {
            p.use = expect(TokenKind::Identifier).text;
            expect(";");                         // ;
        }
        else if (accept("PORT")) {
            parsePort(p);
        }
        else if (accept("END")) {
            expect(pinName);
            break;
        }
        else { tz_.next(); }
    }
    mc.pins.push_back(std::move(p));
}

/* ──────────────────── PORT inside PIN ────────────────────────────────── */
void LefParser::parsePort(Pin& p)
{
    std::string curLayer;

    while (true) {
        if (accept("LAYER")) {
            curLayer = expect(TokenKind::Identifier).text;
            expect(";");                         // ;
        }
        else if (accept("RECT")) {
            PortRect r; r.layer = curLayer.empty() ? "<unk_layer>" : curLayer;
            r.box.xl = toMicron(expect(TokenKind::Number).text);
            r.box.yl = toMicron(expect(TokenKind::Number).text);
            r.box.xh = toMicron(expect(TokenKind::Number).text);
            r.box.yh = toMicron(expect(TokenKind::Number).text);
            expect(";");                         // ;
            p.ports.push_back(std::move(r));
        }
        else if (accept("POLYGON")) {
            Polygon pg; pg.layer = curLayer.empty() ? "<unk_layer>" : curLayer;
            while (tz_.peek().kind == TokenKind::Number)
                pg.pts.push_back(toMicron(tz_.next().text));
            expect(";");                         // ;
            p.polygons.push_back(std::move(pg));
        }
        else if (accept("END")) break;
        else { tz_.next(); }
    }
}

/* ──────────────────── OBS inside MACRO ───────────────────────────────── */
void LefParser::parseObs(Macro& mc)
{
    std::string curLayer;

    while (true) {
        if (accept("LAYER")) {
            curLayer = expect(TokenKind::Identifier).text;
            expect(";");                         // ;
        }
        else if (accept("RECT")) {
            ObsRect r; r.layer = curLayer;
            r.box.xl = toMicron(expect(TokenKind::Number).text);
            r.box.yl = toMicron(expect(TokenKind::Number).text);
            r.box.xh = toMicron(expect(TokenKind::Number).text);
            r.box.yh = toMicron(expect(TokenKind::Number).text);
            expect(";");                         // ;
            mc.obstructions.push_back(std::move(r));
        }
        else if (accept("POLYGON")) {
            Polygon pg; pg.layer = curLayer;
            while (tz_.peek().kind == TokenKind::Number)
                pg.pts.push_back(toMicron(tz_.next().text));
            expect(";");                         // ;
            mc.obsPolygons.push_back(std::move(pg));
        }
        else if (accept("END")) break;
        else { tz_.next(); }
    }
}

} // namespace lef

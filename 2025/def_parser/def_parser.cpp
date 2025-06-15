#include "def_parser.hpp"
#include <stdexcept>
#include <iostream>

DefParser::DefParser(std::istream& in) : tokenizer(in) {}

Token DefParser::expect(TokenKind expected) {
    Token t = tokenizer.next();
    if (t.kind != expected)
        throw std::runtime_error("Unexpected token: " + t.text);
    return t;
}

Design DefParser::parse() {
    std::cout << "Starting DEF parsing...\n";
    Design d;
    parseHeader(d);
    std::cout << "Header parsed. Design name: " << d.name << ", DBU per micron: " << d.dbuPerMicron << "\n";
    parseDieArea(d);
    std::cout << "Die area parsed: (" << d.dieArea.xl << ", " << d.dieArea.yl << ") to (" << d.dieArea.xh << ", " << d.dieArea.yh << ")\n";
    
    Token next = tokenizer.peek();
    
    while (next.kind != TokenKind::EndOfFile) {
        std::cout << "Main loop, next token: '" << next.text << "'\n";
        if (next.text == "ROW") {
            parseRows(d);
        }
        else if (next.text == "TRACKS") {
            parseTracks(d);
        }
        else if (next.text == "COMPONENTS") {
            parseComponents(d);
        }
        else if (next.text == "PINS") {
            parsePins(d);
        }
        else if (next.text == "PINPROPERTIES") {
            parsePinProperties(d);
        }
        else if (next.text == "NETS") {
            parseNets(d);
        }
        else {
            // Skip unknown section
            std::cerr << "Skipping unknown section: " << next.text << "\n";
            tokenizer.next(); // skip the section keyword
            int guard = 0;
            while (true) {
                Token t = tokenizer.peek();
                if (t.kind == TokenKind::EndOfFile) break;
                if (t.text == "ROW" || t.text == "TRACKS" || t.text == "COMPONENTS" || 
                    t.text == "PINS" || t.text == "PINPROPERTIES" || t.text == "NETS") break;
                if (t.text == "END") {
                    tokenizer.next(); // consume END
                    Token maybeSection = tokenizer.next();
                    Token maybeSemi = tokenizer.peek();
                    if (maybeSemi.text == ";") tokenizer.next();
                    break;
                }
                tokenizer.next();
                if (++guard > 100000) throw std::runtime_error("Skip loop guard triggered");
            }
        }
        next = tokenizer.peek();
    }
    
    std::cout << "\nParsing complete:\n";
    std::cout << "Rows: " << d.rows.size() << "\n";
    std::cout << "Tracks: " << d.tracks.size() << "\n";
    std::cout << "Components: " << d.comps.size() << "\n";
    std::cout << "Pins: " << d.pins.size() << "\n";
    std::cout << "Pin Properties: " << d.pinProperties.size() << "\n";
    std::cout << "Nets: " << d.nets.size() << "\n";
    
    return d;
}

void DefParser::parseHeader(Design& d) {
    while (true) {
        Token tok = tokenizer.peek();
        std::cout << "Parsing header section: " << tok.text << "\n";

        if (tok.kind == TokenKind::EndOfFile) {
            throw std::runtime_error("Unexpected end of file in header.");
        }

        if (tok.text == "VERSION") {
            tokenizer.next();
            expect(TokenKind::Number); // e.g. 5.8
            expect(TokenKind::Symbol); // ;
        }
        else if (tok.text == "DIVIDERCHAR" || tok.text == "BUSBITCHARS") {
            tokenizer.next(); // consume keyword
            tokenizer.next(); // consume quoted value (e.g. "/" or "[]")
            expect(TokenKind::Symbol); // ;
        }
        else if (tok.text == "UNITS") {
            tokenizer.next();
            expect(TokenKind::Identifier); // DISTANCE
            expect(TokenKind::Identifier); // MICRONS
            Token dbuToken = expect(TokenKind::Number);
            d.dbuPerMicron = std::stoi(dbuToken.text);
            expect(TokenKind::Symbol); // ;
        }
        else if (tok.text == "DESIGN") {
            tokenizer.next();
            d.name = expect(TokenKind::Identifier).text;
            expect(TokenKind::Symbol); // ;
        }
        else if (tok.text == "PROPERTYDEFINITIONS") {
            tokenizer.next(); // consume PROPERTYDEFINITIONS
            while (true) {
                Token t = tokenizer.next();
                if (t.text == "END" && tokenizer.peek().text == "PROPERTYDEFINITIONS") {
                    tokenizer.next(); // consume PROPERTYDEFINITIONS
                    break;
                }
            }
        }
        else if (tok.text == "DIEAREA") {
            // Let parseDieArea handle this
            break;
        }
        else if (tok.text == "ROW") {
            // Let parseRows handle this
            break;
        }
        else if (tok.text == "TRACKS") {
            // Let parseTracks handle this
            break;
        }
        else if (tok.text == "COMPONENTS") {
            // Let parseComponents handle this
            break;
        }
        else {
            // Fallback: skip unknown section up to ';'
            tokenizer.next(); // consume the unknown token
            std::cerr << "Skipping unknown header section: " << tok.text << "\n";
            int guard = 0;
            while (true) {
                Token t = tokenizer.next();
                if (t.text == ";") break;
                if (t.kind == TokenKind::EndOfFile)
                    throw std::runtime_error("Unexpected EOF while skipping unknown header section");
                if (++guard > 10000)
                    throw std::runtime_error("Header skip loop guard triggered");
            }
        }
    }
}

void DefParser::parseDieArea(Design& d) {
    Token tok = tokenizer.next(); // DIEAREA
    if (tok.text != "DIEAREA")
        throw std::runtime_error("Expected DIEAREA");

    std::vector<Point> corners;

    while (true) {
        Token next = tokenizer.peek();
        if (next.text == ";") {
            tokenizer.next(); // consume ;
            break;
        }

        expect(TokenKind::Symbol); // (
        int x = std::stoi(expect(TokenKind::Number).text);
        int y = std::stoi(expect(TokenKind::Number).text);
        expect(TokenKind::Symbol); // )
        corners.push_back({x, y});
    }

    if (corners.empty())
        throw std::runtime_error("DIEAREA has no points");

    // compute bounding box
    Coord xl = corners[0].x, yl = corners[0].y;
    Coord xh = xl, yh = yl;

    for (const auto& p : corners) {
        if (p.x < xl) xl = p.x;
        if (p.y < yl) yl = p.y;
        if (p.x > xh) xh = p.x;
        if (p.y > yh) yh = p.y;
    }

    d.dieArea = {xl, yl, xh, yh};
}

void DefParser::parseRows(Design& d) {
    while (true) {
        Token tok = tokenizer.peek();
        if (tok.text != "ROW") break;
        tokenizer.next(); // consume ROW

        Row row;
        Token nameTok = tokenizer.next();
        if (nameTok.kind != TokenKind::Identifier) {
            throw std::runtime_error("Expected row name, got: " + nameTok.text);
        }
        row.name = nameTok.text;
        
        Token siteTok = tokenizer.next();
        if (siteTok.kind != TokenKind::Identifier) {
            throw std::runtime_error("Expected site name, got: " + siteTok.text);
        }
        row.siteName = siteTok.text;
        
        row.origin.x = std::stoi(expect(TokenKind::Number).text);
        row.origin.y = std::stoi(expect(TokenKind::Number).text);
        
        row.orient = parseOrient(expect(TokenKind::Identifier).text);
        
        expect(TokenKind::Identifier); // DO
        row.numX = std::stoi(expect(TokenKind::Number).text);
        expect(TokenKind::Identifier); // BY
        row.numY = std::stoi(expect(TokenKind::Number).text);
        expect(TokenKind::Identifier); // STEP
        row.stepX = std::stoi(expect(TokenKind::Number).text);
        row.stepY = std::stoi(expect(TokenKind::Number).text);
        expect(TokenKind::Symbol); // ;
        d.rows.push_back(std::move(row));
    }
}

void DefParser::parseTracks(Design& d) {
    Token tok = tokenizer.next(); // TRACKS
    if (tok.text != "TRACKS")
        throw std::runtime_error("Expected TRACKS");

    Track track;
    track.direction = expect(TokenKind::Identifier).text; // X or Y
    track.start = std::stoi(expect(TokenKind::Number).text);
    expect(TokenKind::Identifier); // DO
    track.numTracks = std::stoi(expect(TokenKind::Number).text);
    expect(TokenKind::Identifier); // STEP
    track.step = std::stoi(expect(TokenKind::Number).text);
    
    expect(TokenKind::Identifier); // LAYER
    while (true) {
        Token t = tokenizer.peek();
        if (t.text == ";") {
            tokenizer.next(); // consume ;
            break;
        }
        track.layers.push_back(expect(TokenKind::Identifier).text);
    }
    d.tracks.push_back(std::move(track));
}

void DefParser::parseComponents(Design& d) {
    Token tok = tokenizer.next(); // COMPONENTS
    if (tok.text != "COMPONENTS")
        throw std::runtime_error("Expected COMPONENTS");

    int declared_count = std::stoi(expect(TokenKind::Number).text);
    expect(TokenKind::Symbol); // ;

    int parsed_count = 0;
    while (true) {
        Token t = tokenizer.peek();
        // std::cout << "[COMPONENTS] Next token: '" << t.text << "' (kind=" << static_cast<int>(t.kind) << ")\n";
        if (t.text == "END") {
            tokenizer.next(); // consume END
            Token compTok = tokenizer.next(); // should be COMPONENTS
            if (compTok.text != "COMPONENTS")
                throw std::runtime_error("Expected COMPONENTS after END");
            // expect(TokenKind::Symbol); // ;
            break;
        }

        Token dash = tokenizer.next();
        if (dash.text != "-") {
            throw std::runtime_error("Expected '-' at start of component, got: " + dash.text);
        }

        Component c;
        c.instName = expect(TokenKind::Identifier).text;
        c.refName  = expect(TokenKind::Identifier).text;

        bool foundPlacement = false;
        while (true) {
            Token t = tokenizer.next();
            // std::cout << "[COMPONENTS]   Inner token: '" << t.text << "' (kind=" << static_cast<int>(t.kind) << ")\n";
            if (t.text == "PLACED" || t.text == "FIXED") {
                c.status = parseStatus(t.text);
                expect(TokenKind::Symbol); // (
                c.pos.x = std::stoi(expect(TokenKind::Number).text);
                c.pos.y = std::stoi(expect(TokenKind::Number).text);
                expect(TokenKind::Symbol); // )
                c.orient = parseOrient(expect(TokenKind::Identifier).text);
                expect(TokenKind::Symbol); // ;
                foundPlacement = true;
                break;
            } else if (t.text == ";") {
                break;
            } else if (t.text == "END") {
                // Handle unexpected END token
                tokenizer.next(); // consume END
                Token compTok = tokenizer.next(); // should be COMPONENTS
                if (compTok.text != "COMPONENTS")
                    throw std::runtime_error("Expected COMPONENTS after END");
                expect(TokenKind::Symbol); // ;
                goto finish_components;
            } else {
                // Skip any unrecognized/optional fields until ';' or section end
                // std::cout << "[COMPONENTS]   Skipping unrecognized token: '" << t.text << "'\n";
                // continue to next token
            }
        }

        if (!foundPlacement) {
            c.status = Status::UNPLACED;
        }

        c.id = d.comps.size();
        d.comps.push_back(std::move(c));
        parsed_count++;
    }
finish_components:
    std::cout << "Parsed " << parsed_count << " components (declared: " << declared_count << ")\n";
    if (parsed_count != declared_count) {
        std::cerr << "Warning: Parsed component count (" << parsed_count << ") does not match declared count (" << declared_count << ")!\n";
    }
}

void DefParser::parsePins(Design& d) {
    Token tok = tokenizer.next(); // PINS
    if (tok.text != "PINS")
        throw std::runtime_error("Expected PINS");

    int count = std::stoi(expect(TokenKind::Number).text);
    expect(TokenKind::Symbol); // ;

    while (true) {
        Token t = tokenizer.peek();
        if (t.text == "END") {
            tokenizer.next(); // consume END
            Token pinsTok = tokenizer.next(); // should be PINS
            if (pinsTok.text != "PINS")
                throw std::runtime_error("Expected PINS after END");
            // expect(TokenKind::Symbol); // ;
            break;
        }

        Token dash = tokenizer.next();
        if (dash.text != "-") {
            throw std::runtime_error("Expected '-' at start of pin, got: " + dash.text);
        }

        Pin pin;
        pin.name = expect(TokenKind::Identifier).text;
        
        expect(TokenKind::Symbol); // +
        expect(TokenKind::Identifier); // NET
        pin.netName = expect(TokenKind::Identifier).text;
        
        expect(TokenKind::Symbol); // +
        expect(TokenKind::Identifier); // DIRECTION
        pin.direction = parseDirection(expect(TokenKind::Identifier).text);
        
        expect(TokenKind::Symbol); // +
        expect(TokenKind::Identifier); // USE
        expect(TokenKind::Identifier); // SIGNAL
        
        expect(TokenKind::Symbol); // +
        expect(TokenKind::Identifier); // LAYER
        pin.layer = expect(TokenKind::Identifier).text;
        
        expect(TokenKind::Symbol); // (
        int x1 = std::stoi(expect(TokenKind::Number).text);
        int y1 = std::stoi(expect(TokenKind::Number).text);
        expect(TokenKind::Symbol); // )
        expect(TokenKind::Symbol); // (
        int x2 = std::stoi(expect(TokenKind::Number).text);
        int y2 = std::stoi(expect(TokenKind::Number).text);
        expect(TokenKind::Symbol); // )
        pin.port = {x1, y1, x2, y2};
        
        expect(TokenKind::Symbol); // +
        expect(TokenKind::Identifier); // PLACED
        expect(TokenKind::Symbol); // (
        pin.pos.x = std::stoi(expect(TokenKind::Number).text);
        pin.pos.y = std::stoi(expect(TokenKind::Number).text);
        expect(TokenKind::Symbol); // )
        pin.orient = parseOrient(expect(TokenKind::Identifier).text);
        expect(TokenKind::Symbol); // ;
        
        d.pins.push_back(std::move(pin));
    }
}

void DefParser::parsePinProperties(Design& d) {
    Token tok = tokenizer.next(); // PINPROPERTIES
    if (tok.text != "PINPROPERTIES")
        throw std::runtime_error("Expected PINPROPERTIES");

    int count = std::stoi(expect(TokenKind::Number).text);
    expect(TokenKind::Symbol); // ;

    while (true) {
        Token t = tokenizer.peek();
        if (t.text == "END") {
            tokenizer.next(); // consume END
            Token propsTok = tokenizer.next(); // should be PINPROPERTIES
            if (propsTok.text != "PINPROPERTIES")
                throw std::runtime_error("Expected PINPROPERTIES after END");
            // expect(TokenKind::Symbol); // ;
            break;
        }

        Token dash = tokenizer.next();
        if (dash.text != "-") {
            throw std::runtime_error("Expected '-' at start of pin property, got: " + dash.text);
        }

        expect(TokenKind::Identifier); // PIN
        PinProperty prop;
        prop.pinName = expect(TokenKind::Identifier).text;
        
        expect(TokenKind::Symbol); // +
        expect(TokenKind::Identifier); // PROPERTY
        prop.propertyName = expect(TokenKind::Identifier).text;
        prop.value = expect(TokenKind::Identifier).text;
        expect(TokenKind::Symbol); // ;
        
        d.pinProperties.push_back(std::move(prop));
    }
}

void DefParser::parseNets(Design& d) {
    Token tok = tokenizer.next(); // NETS
    if (tok.text != "NETS")
        throw std::runtime_error("Expected NETS");

    int count = std::stoi(expect(TokenKind::Number).text);
    expect(TokenKind::Symbol); // ;

    while (true) {
        Token t = tokenizer.peek();
        if (t.text == "END") {
            tokenizer.next(); // consume END
            Token netsTok = tokenizer.next(); // should be NETS
            if (netsTok.text != "NETS")
                throw std::runtime_error("Expected NETS after END");
            // expect(TokenKind::Symbol); // ;
            break;
        }

        Token dash = tokenizer.next();
        if (dash.text != "-") {
            throw std::runtime_error("Expected '-' at start of net, got: " + dash.text);
        }

        Net net;
        // Handle net name that might contain array indices
        Token nameTok = tokenizer.next();
        if (nameTok.kind != TokenKind::Identifier) {
            throw std::runtime_error("Expected net name, got: " + nameTok.text);
        }
        net.name = nameTok.text;
        
        // Parse connections
        while (true) {
            Token next = tokenizer.peek();
            if (next.text == "+") {
                tokenizer.next(); // consume +
                expect(TokenKind::Identifier); // USE
                expect(TokenKind::Identifier); // SIGNAL
                expect(TokenKind::Symbol); // ;
                break;
            }
            
            expect(TokenKind::Symbol); // (
            std::string comp = expect(TokenKind::Identifier).text;
            Token pinTok = tokenizer.next();
            if (pinTok.kind != TokenKind::Identifier) {
                throw std::runtime_error("Expected pin name, got: " + pinTok.text);
            }
            expect(TokenKind::Symbol); // )
            net.connections.push_back(comp + " " + pinTok.text);
        }
        
        d.nets.push_back(std::move(net));
    }
}

Orient DefParser::parseOrient(const std::string& s) {
    if (s == "N") return Orient::N;
    if (s == "S") return Orient::S;
    if (s == "E") return Orient::E;
    if (s == "W") return Orient::W;
    if (s == "FN") return Orient::FN;
    if (s == "FS") return Orient::FS;
    if (s == "FE") return Orient::FE;
    if (s == "FW") return Orient::FW;
    throw std::runtime_error("Unknown orientation: " + s);
}

Status DefParser::parseStatus(const std::string& s) {
    if (s == "PLACED") return Status::PLACED;
    if (s == "FIXED") return Status::FIXED;
    return Status::UNPLACED;
}

Direction DefParser::parseDirection(const std::string& s) {
    if (s == "INPUT") return Direction::INPUT;
    if (s == "OUTPUT") return Direction::OUTPUT;
    if (s == "INOUT") return Direction::INOUT;
    throw std::runtime_error("Unknown direction: " + s);
}

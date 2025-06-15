#include "lef_parser.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2) { std::cerr << "usage: " << argv[0] << " file.lef\n"; return 1; }
    std::ifstream fin(argv[1]);
    lef::LefParser parser(fin);
    lef::Library lib = parser.parse();
    std::cout << "Parsed " << lib.macros.size() << " macros\n";

    //print the names of the macros
    for (const auto& [name, macro] : lib.macros) {
        if (name.find("FSDN") != std::string::npos) {
            std::cout << "Macro: " << name << ", Size: " << macro.sizeX << "x" << macro.sizeY
                      << ", Pins: " << macro.pins.size() << '\n';
        }
    }
}
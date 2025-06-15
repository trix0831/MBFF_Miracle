#include "def_parser.hpp"
#include "../lef_parser/lef_parser.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./def_parser <filename.def>\n";
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin)
    {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    try
    {
        std::cout << "Parsing DEF file: " << argv[1] << "\n";
        DefParser parser(fin);
        Design d = parser.parse();
        std::cout << "Parsed design: " << d.name << "\n";
        std::cout << "Components: " << d.comps.size() << "\n";
        int count = 0;
        for (const auto &c : d.comps)
        {
            std::cout << c.instName << " (" << c.pos.x << ", " << c.pos.y << ")\n";
            count++;
            if (count >= 10)
            {
                std::cout << "Showing first 10 components only...\n";
                break;
            }
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Parse error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

// main.cpp
#include <iostream>
#include <fstream>
#include <iomanip>
#include "verilog_parser.hpp"

using namespace verilog;

/* helper ------------------------------------------------------------------*/
static const char* dir2str(PortDir d) {
    switch (d) {
        case PortDir::INPUT:  return "input";
        case PortDir::OUTPUT: return "output";
        default:              return "inout";
    }
}

/*-------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <design.v>\n";
        return 1;
    }
    std::ifstream fin(argv[1]);
    if (!fin) {
        std::cerr << "Cannot open " << argv[1] << '\n';
        return 1;
    }

    try {
        VerilogParser parser(fin);
        VerilogDesign design = parser.parse();
        const VerilogModule& m = design.mod;

        /* high-level statistics -------------------------------------------*/
        std::cout << "Parsed module `" << m.name << "`\n";
        std::cout << "  #ports     : " << m.ports.size()     << '\n';
        std::cout << "  #nets      : " << m.nets.size()      << '\n';
        std::cout << "  #instances : " << m.instances.size() << '\n';

        /* example ports ---------------------------------------------------*/
        std::cout << "\n=== First "
                  << std::min<std::size_t>(5, m.ports.size()) << " ports ===\n";
        for (std::size_t i = 0; i < m.ports.size() && i < 5; ++i) {
            std::cout << "  [" << i << "] "
                      << m.ports[i].name << " (" << dir2str(m.ports[i].dir) << ")\n";
        }

        /* example nets ----------------------------------------------------*/
        std::cout << "\n=== First "
                  << std::min<std::size_t>(5, m.nets.size()) << " nets ===\n";
        for (std::size_t i = 0; i < m.nets.size() && i < 5; ++i) {
            std::cout << "  [" << i << "] " << m.nets[i].name << '\n';
        }

        /* example instances ----------------------------------------------*/
        std::cout << "\n=== First "
                  << std::min<std::size_t>(5, m.instances.size()) << " instances ===\n";
        for (std::size_t i = 0; i < m.instances.size() && i < 5; ++i) {
            const auto& inst = m.instances[i];
            std::cout << "  [" << i << "] "
                      << inst.cellType << ' ' << inst.instName
                      << "  (pins=" << inst.pin2net.size() << ")\n";

            std::size_t shown = 0;
            for (const auto& kv : inst.pin2net) {
                if (shown++ == 5) break;
                std::cout << "        ." << kv.first << " -> " << kv.second << '\n';
            }
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Parse error : " << ex.what() << '\n';
        return 2;
    }
    return 0;
}

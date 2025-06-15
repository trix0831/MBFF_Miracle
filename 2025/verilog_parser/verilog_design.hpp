#pragma once
/*-----------------------------------------------------------------------------
 *  Pure data model for a flat gate-level Verilog design
 *---------------------------------------------------------------------------*/
#include <string>
#include <vector>
#include <unordered_map>

namespace verilog {

struct VerilogNet { std::string name; };

enum class PortDir { INPUT, OUTPUT, INOUT };
struct VerilogPort {
    std::string name;
    PortDir     dir = PortDir::INPUT;
};

struct VerilogInstance {
    std::string cellType;   // library cell
    std::string instName;   // instance label
    std::unordered_map<std::string, std::string> pin2net; // formal-pin -> net
};

struct VerilogModule {
    std::string                  name;
    std::vector<VerilogPort>     ports;
    std::vector<VerilogNet>      nets;
    std::vector<VerilogInstance> instances;
};

struct VerilogDesign { VerilogModule mod; };

} // namespace verilog

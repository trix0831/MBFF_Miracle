#include <fstream>
#include <iostream>
#include <iomanip>
#include "sdc_parser.hpp"

void printLine(const std::string& title) {
    std::cout << "\n===== " << title << " =====\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.sdc>\n";
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin) {
        std::cerr << "Cannot open file: " << argv[1] << '\n';
        return 2;
    }

    try {
        int count = 0;
        sdc::SdcParser parser(fin);
        sdc::Sdc db = parser.parse();

        // Header Info
        printLine("SDC Version & Units");
        std::cout << "Version      : " << db.version << '\n';
        std::cout << "Time Unit    : " << db.timeUnit << '\n';
        std::cout << "Resistance   : " << db.resistanceUnit << '\n';
        std::cout << "Capacitance  : " << db.capacitanceUnit << '\n';
        std::cout << "Voltage      : " << db.voltageUnit << '\n';
        std::cout << "Current      : " << db.currentUnit << '\n';

        // Clocks
        printLine("Clocks");
        for (const auto& clk : db.clocks) {
            std::cout << "Clock '" << clk.name << "'\n";
            std::cout << "  Period     : " << clk.period << '\n';
            std::cout << "  Waveform   : {" << clk.waveform.first << ", " << clk.waveform.second << "}\n";
            std::cout << "  PortExpr   : " << clk.portExpr << '\n';
        }

        // Loads
        printLine("Loads");
        for (const auto& l : db.loads) {
            std::cout << "Load on port: " << l.portExpr << " = " << l.pinLoad << '\n';
            count++;
            if (count > 5){
                count = 0;
                break;
            }
        }

        // Latencies
        printLine("Clock Latency");
        for (const auto& lat : db.latencies) {
            std::cout << "Latency = " << lat.value << " on " << lat.clockExpr << '\n';
        }

        // Uncertainty
        printLine("Clock Uncertainty");
        for (const auto& u : db.uncertainties) {
            std::cout << "Uncertainty = " << u.value << " on " << u.clockExpr << '\n';
        }

        // Transition
        printLine("Clock Transition");
        for (const auto& t : db.transitions) {
            std::cout << "Transition = " << t.value << " on " << t.clockExpr << '\n';
            count++;
            if (count > 5){
                count = 0;
                break;
            }
        }

        
        // Input Delay
        printLine("Input Delay");
        for (const auto& in : db.inputDelays) {
            std::cout << "Input delay " << in.delay << " on port " << in.portExpr << " clock " << in.clockExpr << '\n';
            count++;
            if (count > 5){
                count = 0;
                break;
            }
        }

        // Output Delay
        printLine("Output Delay");
        for (const auto& out : db.outputDelays) {
            std::cout << "Output delay " << out.delay << " on port " << out.portExpr << " clock " << out.clockExpr << '\n';
            count++;
            if (count > 5){
                break;
            }
        }

        // Limits
        printLine("Limits");
        for (const auto& lim : db.limits) {
            std::cout << lim.limitExpr << " = " << lim.val << " on target " << lim.targetExpr << '\n';
        }

    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << '\n';
        return 3;
    }

    return 0;
}

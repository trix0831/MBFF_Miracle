#include "weight_parser.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_weight_file>\n";
        return 1;
    }

    WeightData data;
    if (!parseWeightFile(argv[1], data)) {
        std::cerr << "Error: Failed to parse weight file.\n";
        return 1;
    }

    std::cout << "Alpha: " << data.alpha << "\n";
    std::cout << "Beta: " << data.beta << "\n";
    std::cout << "Gamma: " << data.gamma << "\n";
    std::cout << "TNS: " << data.tns << "\n";
    std::cout << "TPO: " << data.tpo << "\n";
    std::cout << "Area: " << data.area << "\n";

    return 0;
}

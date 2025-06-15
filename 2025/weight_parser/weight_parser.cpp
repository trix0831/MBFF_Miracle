#include "weight_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

bool parseWeightFile(const std::string& path, WeightData& data) {
    std::ifstream fin(path);
    if (!fin) {
        std::cerr << "Failed to open weight file: " << path << "\n";
        return false;
    }

    std::string key;
    double value;
    while (fin >> key >> value) {
        if (key == "Alpha") data.alpha = value;
        else if (key == "Beta") data.beta = value;
        else if (key == "Gamma") data.gamma = value;
        else if (key == "TNS") data.tns = value;
        else if (key == "TPO") data.tpo = value;
        else if (key == "Area") data.area = value;
        else {
            std::cerr << "Unknown key: " << key << "\n";
            return false;
        }
    }

    return true;
}

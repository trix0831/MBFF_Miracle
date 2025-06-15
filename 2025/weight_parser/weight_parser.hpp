#pragma once
#include <string>

struct WeightData {
    double alpha = 0.0;
    double beta = 0.0;
    double gamma = 0.0;
    double tns = 0.0;
    double tpo = 0.0;
    double area = 0.0;
};

bool parseWeightFile(const std::string& path, WeightData& data);

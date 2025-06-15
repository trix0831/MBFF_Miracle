/*
 * Load the JSON produced by liberty_to_json.py and perform
 * simple sanity checks on the content.
 *
 * Compile with:
 *   g++ -std=c++17 -O2 main.cpp -o libcheck
 * Make sure nlohmann/json.hpp is in your include path.
 *
 * Run:
 *   ./libcheck [lib_data.json]
 */

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static bool ends_with(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main(int argc, char* argv[]) {
    const std::string json_path = argc > 1 ? argv[1] : "lib_data.json";

    std::ifstream in(json_path);
    if (!in) {
        std::cerr << "Error: cannot open " << json_path << '\n';
        return 1;
    }

    json root;
    in >> root;

    /* --- Test 1: at least one *_unit key present ------------------------ */
    bool found_unit = false;
    for (const auto& [key, value] : root.items()) {
        if (ends_with(key, "_unit")) {
            found_unit = true;
            break;
        }
    }
    assert(found_unit && "No *_unit found in JSON");

    /* --- Test 2: the root must contain a group list --------------------- */
    assert(root.contains("group") && "Root JSON has no 'group' key");
    const auto& groups = root["group"];
    assert(groups.is_array() && "root['group'] is not an array");

    /* --- Test 3: at least one cell group exists ------------------------- */
    std::size_t cell_count = 0;
    for (const auto& g : groups) {
        if (g.value("type", "") == "cell")
            ++cell_count;
    }
    assert(cell_count > 0 && "No cell groups found in Liberty JSON");

    /* --- Report --------------------------------------------------------- */
    std::cout << "JSON file: " << json_path << '\n';
    std::cout << "  Units present: " << (found_unit ? "yes" : "no") << '\n';
    std::cout << "  Total groups : " << groups.size() << '\n';
    std::cout << "  Cell groups  : " << cell_count << '\n';
    std::cout << "All sanity checks passed ✅\n";
    return 0;
}

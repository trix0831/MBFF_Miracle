#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include "MBFFOptimizer.h"
#include "visualizer.h"

using namespace std;
int main(int argc, char **argv)
{
    fstream input, output;
    if (argc == 3)
    {
        input.open(argv[1], ios::in);
        // output.open(argv[2], ios::out);
        if (!input)
        {
            cerr << "Cannot open the input file \"" << argv[1]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!output)
        {
            cerr << "Cannot open the output file \"" << argv[2]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
    }
    else
    {
        cerr << "Usage: ./fm <input file> <output file>" << endl;
        exit(1);
    }

    std::string inputPath = argv[1];
    // Step 1: Extract file name (without directory)
    size_t lastSlash = inputPath.find_last_of("/\\");
    std::string fileName = (lastSlash == std::string::npos) ? inputPath : inputPath.substr(lastSlash + 1);

    // Step 2: Remove extension
    size_t lastDot = fileName.find_last_of('.');
    std::string baseName = (lastDot == std::string::npos) ? fileName : fileName.substr(0, lastDot);

    MBFFOptimizer *optimizer = new MBFFOptimizer(input, argv[2]);
    plotInit(optimizer->dieWidth(), optimizer->dieHeight(),
         optimizer->name2pInstances_ff(), optimizer->name2pInstances_gate(), baseName);
    cout << "check" << endl;

    optimizer->init_occupied();

    optimizer->algorithm(baseName);

    plotMerge(optimizer->dieWidth(), optimizer->dieHeight(),
         optimizer->name2pInstances_ff(), optimizer->name2pInstances_gate(), optimizer->_mergedInstances, baseName, 0);

    cout << "total runtime: " << (double)clock() / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}

# one-shot build
g++ -std=c++17 -O2 main.cpp                  \
    verilog_tokenizer.cpp verilog_parser.cpp \
    -o vparse

# usage
./vparse ../../testcase1/testcase1.v
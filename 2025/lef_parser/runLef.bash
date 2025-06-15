g++ -std=c++17 -O2 -Wall \
    lef_tokenizer.cpp lef_parser.cpp main.cpp \
    -o lef_parser

./lef_parser ../../testcase1/SNPSHOPT25/lef/snps25hopt.lef 
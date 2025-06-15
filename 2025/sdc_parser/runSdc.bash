rm -f sdc_test
g++ -std=c++17 -Wall -O2 \
      main.cpp \
      sdc_parser.cpp \
      sdc_tokenizer.cpp \
      -o sdc_test

./sdc_test ../../testcase1/testcase1.sdc
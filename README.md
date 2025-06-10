# eda_CADb
eda makes me suffer..

## Introduction to the file structure
For the directory bin, it contatins the execution file of the main.cpp.

In the src directory, CellLibrary, DisSet,Graph,Instance,Net,Pin,PlacementRow,Point are file for constructing the data structure used in the main algorithm.

There is a MBFFOptimizer file that can call above header file and build the data structure when main file have received input and output file. The algorithm we use is implemented in MBFFOptimizer including syntheisze, find_feasible_region ... and so on. 

The testcase directory including sample case and testcase provided by ICCAD Contest 2024.

The util directory including the sanity provided by ICCAD Contest 2024 for checking if the placement is available. And there is another visualizer directory contatins python file that can show the placement result of our algorithm.

The output directory is the result that generated from us in our local computer.

The makefile for compile main.cpp

The reference paper are the paper we read for solving this algorithm

### Getting Started
under the directory of EDA_CADb
```
make
./bin/mbff <input_file> <output_file>
```
>For example, under the directory of EDA_CADb
```
./bin/mbff ./testcase/testcase1/testcase1.txt ./output/out
```
### Visualize 
under the directory of EDA_CADb
```bash
python3 ./util/visualizer/plot.py <input_file> <output_file>
```
>For example
 ```bash
 python3 ./util/visualizer/plot_result.py ./testcase/testcase1/testcase1.txt ./output/out
 ```
### Check correctness
under the directory of EDA_CADb
```bash
./util/sanity/sanity <input_file> <output_file>
```
>For example
 ```bash
 ./util/sanity/sanity ./testcase/testcase1/testcase1.txt ./output/out
 ```

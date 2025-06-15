# MBFF_Miracle
ICCAD Contest Problem B: Power and Timing Optimization Using Multibit Flip-Flop

Team Number: 26

Team Member: B11901158 Li-Cheng Hsu, B11901153 Hao-Qun Lin, B11901078 Wei-Lung Lin, B11502073 Tzu-En Chiu

## Introduction
With the advancement of process nodes, reducing power
and area while preserving timing is crucial. Flip-flops consume
significant dynamic power and area, making them prime can-
didates for optimization. We propose an MBFF optimization
framework that replaces multiple 1-bit FFs with legal MBFF
instances while maintaining design correctness.

## How to run the code
Install Cairo, which is the plotting library
```bash
sudo apt install libcairo2-dev
```
In the "MBFF_Miracle" directory, type:
```bash 
make
```
to make the binary file mbff under directory "bin"

To run the program, run:
```bash
bin/mbff <input-file> <output-file>
```
For example:
```bash
bin/mbff testcase/testcase1_0812.txt output/out1
```
For this case, you should see the final result in the output/out1 and the visualization result under the directory "pic"

## Evalutation
To get the evaluation score (Evaluator provided by ICCAD Contest 2024), run
```
./main <input-file> <output-file>
```
For example, run
```bash
./main testcase/testcase1_0812.txt output/out1
```

## Bonus: Generate gif file for the calculating process
To generate the gif file, you must collect a series of image while calculating 
```
python3 gifGenerator.py <directory which store image>
```
For example,
```
python3 gifGenerator.py pic/testcase1_0812
```

## 2025 Part
In the report we have mentioned that due to VM problem, we don't have access to Fusion Compiler for a long time. So after disucussion we decided to use the testcase from 2024. We have implemented the parser for the 2025 case and all of them is placed under the directory "2025".

You can run the visualization part by:
```
bash visualizer.sh
visualizer
```
But this command should not work if you don't have the testcase provided by the ICCAD Contest 2025, but we can't upload it due to its size and the Non-Disclosure Agreement of Synopsys. The result of the program would be like "2025/chip_layout.png".
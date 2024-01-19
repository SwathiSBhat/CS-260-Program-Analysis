# cs260 w24 assignment 0

__This is not a graded assignment__ and you will not turn it in. It is intended purely to help you prepare the necessary infrastructure for the real assignments.

The goal is to read in a program in LIR format (as described in the accompanying file `lir-description.md`) from a file and store it in a data structure that allows you to easily iterate over functions, basic blocks, and instructions. This is something that you will need to do for all the upcoming assignments in this class. The LIR format is intended to be very easy to parse; the only context-free part is types (see the grammar in `lir-description.md`) and those are still designed to be trivial to parse.

Optionally, if it's easier for you, the LIR programs are also provided in JSON format. These are the exact same programs, just pre-parsed and then serialized as JSON objects. You can choose to __either__ parse the LIR format __or__ read in the JSON format; you __don't__ need to do both.

A set of LIR programs is contained in the accompanying file `tests.zip`. To help determine whether you read in the program correctly, for each program file `<name>.lir` there is a associated file `<name>.stats` that prints out a set of statistics---you can compute the same statistics from your own data structure and compare to make sure they are the same. To be clear, the statistics don't matter themselves, they are just a way to help you determine if you can read in and iterate over programs correctly.

The statistics are:

- Number of fields across all struct types
- Number of functions that return a value
- Number of function parameters
- Number of local variables
- Number of basic blocks
- Number of instructions
- Number of terminals
- Number of locals and globals with int type
- Number of locals and globals with struct type
- Number of locals and globals with pointer to int type
- Number of locals and globals with pointer to struct type
- Number of locals and globals with pointer to function type
- Number of locals and globals with pointer to pointer type

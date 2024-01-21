#!/bin/bash

# bash script to run assn0 on all tests in tests/ directory
# usage: ./parser_test.sh
for input_file in tests/*.lir.json; do
    base_name=$(basename "$input_file" .lir.json)
    # expected_output_file="tests/$base_name.stats"
    echo "------------------------- Running test for $input_file -------------------------"
    ./assn0 $input_file > "./parser_output.txt"
    echo "------------------------- End of running test for $input_file -------------------------"
done
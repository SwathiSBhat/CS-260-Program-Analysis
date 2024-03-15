#!/bin/bash

# bash script to run assn0 on all tests in tests/ directory
# usage: ./parser_test.sh

# exit when any command fails
set -e

for input_file in lir_json/*.lir.json; do
    base_name=$(basename "$input_file" .lir.json)
    # expected_output_file="tests/$base_name.stats"
    echo "------------------------- Running test for $input_file -------------------------"
    ./run-constants-analysis.sh $input_file $input_file "main" > "./parser_output.txt"
    echo "------------------------- End of running test for $input_file -------------------------"
done
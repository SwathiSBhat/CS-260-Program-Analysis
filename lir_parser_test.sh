#!/bin/bash

for input_file in tests/*.lir.json; do
    base_name=$(basename "$input_file" .lir.json)
    expected_output_file="tests/$base_name.stats"
    output=$(python3 "lir_parser.py" "$input_file")
    expected_output=$(cat "$expected_output_file")

    if [ "$output" == "$expected_output" ]; then
        echo "$base_name: Passed"
    else
        echo "$base_name: Failed"
    fi
done

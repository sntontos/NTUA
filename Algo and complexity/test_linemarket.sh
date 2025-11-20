#!/bin/bash

# Compile the C++ file
g++ -o linemarket linemarket.cpp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Compilation successful. Running tests..."

# Directory containing test cases
TEST_DIR="lab01-2/linemarket"

# Loop through all input files in numeric order
for number in $(ls "$TEST_DIR"/input*.txt | sed -E 's/.*input([0-9]+)\.txt$/\1/' | sort -n); do
    input_file="$TEST_DIR/input$number.txt"
    
    # Construct the expected output filename
    expected_output_file="$TEST_DIR/output$number.txt"
    
    # Run the program with the input file
    # Using < redirect to feed input file to stdin
    ./linemarket < "$input_file" > temp_output.txt
    
    # Compare output with expected output
    # Ignoring trailing whitespace with -w and -Z (if supported, otherwise just -w)
    if diff -w temp_output.txt "$expected_output_file" > /dev/null; then
        echo "Test $number: PASS"
    else
        echo "Test $number: FAIL"
        echo "Expected:"
        cat "$expected_output_file"
        echo "Got:"
        cat temp_output.txt
        echo "--------------------------------"
    fi
done

# Clean up
rm linemarket temp_output.txt

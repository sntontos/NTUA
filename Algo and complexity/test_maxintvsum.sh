#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compile the program
echo "Compiling maxintvsum.cpp..."
g++ -std=c++11 -o maxintvsum maxintvsum.cpp
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful!${NC}\n"

# Test counters
passed=0
failed=0

# Test all 20 inputs
for i in {1..20}; do
    input_file="lab01-2/maxintvsum/input${i}.txt"
    output_file="lab01-2/maxintvsum/output${i}.txt"
    
    # Check if files exist
    if [ ! -f "$input_file" ]; then
        echo -e "${YELLOW}Test $i: Input file not found, skipping...${NC}"
        continue
    fi
    
    if [ ! -f "$output_file" ]; then
        echo -e "${YELLOW}Test $i: Output file not found, skipping...${NC}"
        continue
    fi
    
    # Read expected output
    expected=$(cat "$output_file")
    
    # Create a temporary copy with modified input file
    cp maxintvsum.cpp maxintvsum_temp.cpp
    sed -i.bak "s|lab01-2/maxintvsum/input[0-9]*.txt|$input_file|g" maxintvsum_temp.cpp
    
    # Compile the temporary version
    g++ -std=c++11 -o maxintvsum_test maxintvsum_temp.cpp 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ Test $i: Compilation failed${NC}"
        ((failed++))
        rm -f maxintvsum_temp.cpp maxintvsum_temp.cpp.bak maxintvsum_test
        continue
    fi
    
    # Run the program and capture output
    actual=$(./maxintvsum_test 2>&1)
    
    # Compare results (normalize whitespace)
    expected_normalized=$(echo "$expected" | tr -d '\r' | sed '/^$/d')
    actual_normalized=$(echo "$actual" | tr -d '\r' | sed '/^$/d')
    
    if [ "$actual_normalized" == "$expected_normalized" ]; then
        echo -e "${GREEN}✓ Test $i: PASSED${NC}"
        ((passed++))
    else
        echo -e "${RED}✗ Test $i: FAILED${NC}"
        echo "Expected:"
        echo "$expected_normalized" | head -10
        echo "Got:"
        echo "$actual_normalized" | head -10
        echo ""
        ((failed++))
    fi
    
    # Cleanup temporary files
    rm -f maxintvsum_temp.cpp maxintvsum_temp.cpp.bak maxintvsum_test
done

# Summary
echo -e "\n${YELLOW}=== Test Summary ===${NC}"
echo -e "Passed: ${GREEN}$passed${NC}"
echo -e "Failed: ${RED}$failed${NC}"
total=$((passed + failed))
echo -e "Total:  $total"

if [ $failed -eq 0 ] && [ $total -gt 0 ]; then
    echo -e "\n${GREEN}All tests passed! 🎉${NC}"
elif [ $total -eq 0 ]; then
    echo -e "\n${YELLOW}No tests were run.${NC}"
else
    echo -e "\n${RED}Some tests failed. Please review.${NC}"
fi

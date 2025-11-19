#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compile the program
echo "Compiling first.cpp..."
g++ -std=c++11 -o first first.cpp
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
    input_file="lab01-2/linemarket/input${i}.txt"
    output_file="lab01-2/linemarket/output${i}.txt"
    
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
    expected=$(cat "$output_file" | tr -d '\r\n' | tr -d ' ')
    
    # Modify first.cpp to use this input file temporarily
    sed -i.bak "s|lab01-2/linemarket/input[0-9]*.txt|$input_file|g" first.cpp
    
    # Recompile
    g++ -std=c++11 -o first first.cpp 2>/dev/null
    
    # Run the program and capture output
    actual=$(./first | tr -d '\r\n' | tr -d ' ')
    
    # Compare results
    if [ "$actual" == "$expected" ]; then
        echo -e "${GREEN}✓ Test $i: PASSED${NC} (Expected: $expected, Got: $actual)"
        ((passed++))
    else
        echo -e "${RED}✗ Test $i: FAILED${NC} (Expected: $expected, Got: $actual)"
        ((failed++))
    fi
done

# Restore original first.cpp
mv first.cpp.bak first.cpp 2>/dev/null

# Summary
echo -e "\n${YELLOW}=== Test Summary ===${NC}"
echo -e "Passed: ${GREEN}$passed${NC}"
echo -e "Failed: ${RED}$failed${NC}"
total=$((passed + failed))
echo -e "Total:  $total"

if [ $failed -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed! 🎉${NC}"
else
    echo -e "\n${RED}Some tests failed. Please review.${NC}"
fi

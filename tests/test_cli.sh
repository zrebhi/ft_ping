#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

PING_BIN="./ft_ping"
FAILS=0

# Ensure the binary exists
if [ ! -f "$PING_BIN" ]; then
    echo -e "${RED}Error: $PING_BIN not found. Run 'make' first.${NC}"
    exit 1
fi

echo "========================================"
echo "      Running ft_ping CLI Tests         "
echo "========================================"

# Helper function to run a test
# Usage: run_test "Test Name" "args" expected_exit_code "expected_output_string"
run_test() {
    local test_name="$1"
    local args="$2"
    local expected_code=$3
    local expected_str="$4"

    # Run the command, capture stdout and stderr together
    local output
    output=$($PING_BIN $args 2>&1)
    local exit_code=$?

    if [ $exit_code -eq $expected_code ] && [[ "$output" == *"$expected_str"* ]]; then
        echo -e "${GREEN}[PASS]${NC} $test_name"
    else
        echo -e "${RED}[FAIL]${NC} $test_name"
        echo "       Expected Exit Code: $expected_code, Got: $exit_code"
        echo "       Expected String: '$expected_str'"
        echo "       Actual Output: '$output'"
        FAILS=$((FAILS+1))
    fi
}

# --- Test Cases ---

# Test 1: Help Flag
run_test "Help Flag (-?)" "-?" 0 "Usage: ping [OPTION...] HOST ..."

# Test 2: Missing Host
run_test "Missing Host" "" 64 "ping: missing host operand"

# Test 3: Invalid Option
run_test "Invalid Option (-x)" "-x" 64 "ping: invalid option -- 'x'"

# Test 4: Valid Setup (Verbose + Host)
# Note: Since our ping doesn't loop yet, it just prints the debug string and exits 0
run_test "Valid Args (-v google.com)" "-v google.com" 0 "Target: google.com, Verbose: ON"

echo "========================================"
if [ $FAILS -eq 0 ]; then
    echo -e "${GREEN}All CLI tests passed successfully!${NC}"
    exit 0
else
    echo -e "${RED}$FAILS test(s) failed.${NC}"
    exit 1
fi
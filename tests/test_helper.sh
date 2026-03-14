#!/bin/bash

# --- Shared Variables ---
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'
PING_BIN="./ft_ping"
FAILS=0

# --- Binary Check ---
if [ ! -f "$PING_BIN" ]; then
    echo -e "${RED}Error: $PING_BIN not found. Run 'make' first.${NC}"
    exit 1
fi

run_test() {
    local test_name="$1"
    local args="$2"
    local expected_code=$3
    local expected_str="$4"

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

finish_tests() {
    local suite_name="$1"
    echo "========================================"
    if [ $FAILS -eq 0 ]; then
        echo -e "${GREEN}All $suite_name tests passed successfully!${NC}"
        exit 0
    else
        echo -e "${RED}$FAILS $suite_name test(s) failed.${NC}"
        exit 1
    fi
}

require_root() {
    local suite_name="$1"
    if [ "$EUID" -ne 0 ]; then
        echo -e "${YELLOW}[SKIP]${NC} $suite_name tests require root privileges."
        echo "       Tip: Run 'sudo make test' to execute this suite."
        exit 0 # Exit cleanly so the Makefile continues to the next script
    fi
}

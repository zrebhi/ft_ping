#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Valgrind Tests    "
echo "========================================"

require_root "Valgrind"

run_valgrind_test() {
    local test_name="$1"
    local args="$2"
    
    # Capture Valgrind's output into a variable. 
    # We remove --preserve-status and --error-exitcode as we don't need them anymore.
    local valgrind_out
    valgrind_out=$(timeout 2 valgrind --leak-check=full $PING_BIN $args 2>&1)

    echo -e "${BLUE}▶ [TEST]${NC} ${test_name} (Valgrind)"

    # Grep the output for the exact success string
    if echo "$valgrind_out" | grep -q "ERROR SUMMARY: 0 errors"; then
        echo -e "${GREEN}[PASS]${NC} $test_name (No leaks)"
    else
        echo -e "${RED}[FAIL]${NC} $test_name (Memory leak detected!)"
        # Print the specific leak line for easy debugging
        echo "$valgrind_out" | grep "definitely lost:" | sed 's/^/       /'
        FAILS=$((FAILS+1))
    fi
}


run_valgrind_test "Valid Host" "google.com"
run_valgrind_test "Valid Host (IP)" "8.8.8.8"
run_valgrind_test "Invalid Host" "non.existent.domain.42"
run_valgrind_test "Help Menu" "-?"

finish_tests "Valgrind"

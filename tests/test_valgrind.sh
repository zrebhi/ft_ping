#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Valgrind Tests    "
echo "========================================"

require_root "Valgrind"

run_valgrind_test() {
    local test_name="$1"
    local args="$2"
    
    # Run valgrind quietly. We only care if it triggers exit code 42.
    valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=42 $PING_BIN $args > /dev/null 2>&1
    local exit_code=$?

    if [ $exit_code -ne 42 ]; then
        echo -e "${GREEN}[PASS]${NC} $test_name (No leaks)"
    else
        echo -e "${RED}[FAIL]${NC} $test_name (Memory leak detected!)"
        echo "       Run 'valgrind --leak-check=full $PING_BIN $args' manually to see details."
        FAILS=$((FAILS+1))
    fi
}


run_valgrind_test "Valid Host" "google.com"
run_valgrind_test "Valid Host (IP)" "8.8.8.8"
run_valgrind_test "Invalid Host" "non.existent.domain.42"
run_valgrind_test "Help Menu" "-?"

finish_tests "Valgrind"

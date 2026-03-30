#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Signal Tests      "
echo "========================================"

require_root "Signals"

# Test 1: Standard literal string check (is_regex defaults to 0)
run_test "Graceful Exit on SIGINT (Ctrl+C)" "8.8.8.8" 0 "ping statistics ---" 2 "-s INT"

# Test 2: STRICT REGEX - Matches: "X packets transmitted, Y packets received, Z% packet loss"
LOSS_REGEX="[0-9]+ packets transmitted, [0-9]+ packets received, [0-9]+% packet loss"
run_test "Strict Match: Packet Loss Engine" "8.8.8.8" 0 "$LOSS_REGEX" 2 "-s INT" 1

# Test 3: STRICT REGEX - Matches: "round-trip min/avg/max/stddev = X.XXX/Y.YYY/Z.ZZZ/A.AAA ms"
MATH_REGEX="round-trip min/avg/max/stddev = [0-9]+\.[0-9]{3}/[0-9]+\.[0-9]{3}/[0-9]+\.[0-9]{3}/[0-9]+\.[0-9]{3} ms"
run_test "Strict Match: Math Engine (stddev)" "8.8.8.8" 0 "$MATH_REGEX" 2 "-s INT" 1
finish_tests "Signals"

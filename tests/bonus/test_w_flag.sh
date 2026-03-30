#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "    Running ft_ping Timeout (-w) Tests  "
echo "========================================"

require_root "Timeout Bonus"

# Parse limits and edge cases
run_test "Timeout 0 (Too small)" "-w 0 8.8.8.8" 64 "ping: option value too small: 0" 1 "" 0
run_test "Timeout -5 (Underflow)" "-w -5 8.8.8.8" 64 "ping: option value too big: -5" 1 "" 0
run_test "Timeout garbage" "-w abc 8.8.8.8" 64 "ping: invalid value (\`abc' near \`abc')" 1 "" 0
run_test "Timeout missing" "-w" 64 "ping: option requires an argument -- 'w'" 1 "" 0

# Execution testing
# -w 1 should exit cleanly with 0 after 1 second, printing stats
run_test "Timeout 1s execution" "-w 1 8.8.8.8" 0 "ping statistics" 3 "" 0
run_test "Timeout long flag space" "--timeout 1 8.8.8.8" 0 "ping statistics" 3 "" 0

# Unreachable host: should transmit packets, receive 0, hit timeout, and exit 1
run_test "Timeout 1s Unreachable" "-w 1 192.0.2.1" 1 "100% packet loss" 3 "" 0

finish_tests "Timeout Bonus"

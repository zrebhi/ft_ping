#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Signal Tests      "
echo "========================================"

require_root "Signals"

# Test 1: Verify SIGINT (Ctrl+C) triggers the graceful exit banner
# We use 'timeout -s INT 2' to send a SIGINT exactly 2 seconds into the execution
run_test "Graceful Exit on SIGINT (Ctrl+C)" "google.com" 0 "ping statistics ---" 2 "-s INT"

finish_tests "Signals"
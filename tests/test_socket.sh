#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Socket Tests      "
echo "========================================"

# Test 1: Running without sudo should fail gracefully
# Note: We enforce running this specific test WITHOUT sudo
if [ "$EUID" -eq 0 ]; then
    echo -e "${YELLOW}[SKIP]${NC} Socket Non-Root (Cannot test non-root behavior while running as root)"
else
    run_test "Socket Non-Root" "google.com" 1 "ping: socket: Operation not permitted"
fi

finish_tests "Socket"

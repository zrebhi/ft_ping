#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Receive Tests     "
echo "========================================"

require_root "Receive"

# Test 1: Verify we catch an ICMP Echo Reply (Type 0)
run_test "Receive ICMP Reply (google.com)" "google.com" 0 "[DEBUG] Received packet, ICMP Type: 0" 3

finish_tests "Receive"

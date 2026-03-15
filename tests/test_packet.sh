#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Packet Tests      "
echo "========================================"

require_root "Packet"

# Test 1: Verify the packet is successfully forged and sent
# We expect exit code 0 and our temporary debug message
run_test "Forge & Send ICMP (google.com)" "google.com" 0 "Fired 64 bytes into the network"

finish_tests "Packet"

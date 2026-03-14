#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping DNS Tests         "
echo "========================================"

run_test "Valid Host (google.com)" "google.com" 0 "PING google.com"
run_test "Valid IP (8.8.8.8)" "8.8.8.8" 0 "PING 8.8.8.8 (8.8.8.8): 56 data bytes"
run_test "Invalid Host" "non.existent.domain.42" 1 "ping: unknown host"

finish_tests "DNS"

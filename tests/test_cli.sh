#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping CLI Tests         "
echo "========================================"

run_test "Help Flag (-?)" "-?" 0 "Usage: ping [OPTION...] HOST ..."
run_test "Missing Host" "" 64 "ping: missing host operand"
run_test "Invalid Option (-x)" "-x" 64 "ping: invalid option -- 'x'"

finish_tests "CLI"

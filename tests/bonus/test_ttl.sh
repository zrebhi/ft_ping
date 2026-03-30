#!/bin/bash

# Source your excellent helper script
source ./tests/test_helper.sh


echo "========================================"
echo "      Running ft_ping TTL Tests         "
echo "========================================"

require_root "TTL Bonus"

# Signature: run_test "name" "args" expected_code "expected_str" duration "sig_flag" is_regex
run_test "TTL 0 (Too small)" "--ttl 0 8.8.8.8" 64 "ping: option value too small: 0" 1 "" 0
run_test "TTL 256 (Too big)" "--ttl 256 8.8.8.8" 64 "ping: option value too big: 256" 1 "" 0
run_test "TTL -5 (Underflow)" "--ttl -5 8.8.8.8" 64 "ping: option value too big: -5" 1 "" 0
run_test "TTL abc (Garbage)" "--ttl=abc 8.8.8.8" 64 "ping: invalid value (\`abc' near \`abc')" 1 "" 0
run_test "TTL missing arg" "--ttl" 64 "ping: option '--ttl' requires an argument" 1 "" 0
run_test "TTL empty end" "8.8.8.8 --ttl" 64 "ping: option '--ttl' requires an argument" 1 "" 0
run_test "End of options bypass" "-- --ttl 64 8.8.8.8" 1 "ping: unknown host" 2 "" 0

# Here we pass "-s INT" as the 6th argument so timeout cleanly interrupts ft_ping

run_test "TTL 64 Normal" "--ttl 64 8.8.8.8" 0 "ping statistics" 2 "-s INT" 0
run_test "TTL=128 Equals" "--ttl=128 8.8.8.8" 0 "ping statistics" 2 "-s INT" 0

# These tests validate that the TTL was actually applied to the socket by checking for the ICMP error
run_test "TTL 1 Exceeded" "--ttl 1 8.8.8.8" 0 "Time to live exceeded" 2 "-s INT" 0
run_test "TTL 1 Verbose Dump" "--verbose --ttl 1 8.8.8.8" 0 "IP Hdr Dump" 2 "-s INT" 0

finish_tests "TTL Bonus"
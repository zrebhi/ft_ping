#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "  Running ft_ping Ignore Routing Tests  "
echo "========================================"

# We test against 127.0.0.1 because it is an attached loopback network,
# so the -r flag should successfully ping it without routing errors.
run_test "Long Flag --ignore-routing" "--ignore-routing 127.0.0.1" 0 "PING 127.0.0.1"
run_test "Short Flag -r" "-r 127.0.0.1" 0 "PING 127.0.0.1"
run_test "Concatenated Short Flags -vr" "-vr 127.0.0.1" 0 "PING 127.0.0.1"

# We test against an external IP (8.8.8.8). Since it is not on the local 
# attached network, SO_DONTROUTE should cause sendto() to fail.
# Note: inetutils-2.0 prints "ping: sendto: Network is unreachable"
run_test "Flag -r to external host (should fail)" "-r 8.8.8.8" 124 "Network is unreachable" 2 "" 0

finish_tests "Bonus -r Flag CLI"
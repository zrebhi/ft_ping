#!/bin/bash

source ./tests/test_helper.sh

echo -e "${YELLOW}--- Running Bonus -r (Ignore Routing) Tests ---${NC}"

# We test against 127.0.0.1 because it is an attached loopback network,
# so the -r flag should successfully ping it without routing errors.
run_test "Long Flag --ignore-routing" "--ignore-routing 127.0.0.1" 0 "PING 127.0.0.1"
run_test "Short Flag -r" "-r 127.0.0.1" 0 "PING 127.0.0.1"
run_test "Concatenated Short Flags -vr" "-vr 127.0.0.1" 0 "PING 127.0.0.1"

finish_tests "Bonus -r Flag CLI"
#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Quiet (-q) Tests  "
echo "========================================"

require_root "Quiet Bonus"

# Execution testing
# Combine with -c 2 so it finishes cleanly. We expect exit code 0 and the final statistics.
run_test "Quiet Execution (-q)" "-q -c 2 8.8.8.8" 0 "ping statistics" 3 "" 0
run_test "Quiet Execution (--quiet)" "--quiet -c 2 8.8.8.8" 0 "ping statistics" 3 "" 0

# Test conflict/garbage short flags (e.g. providing an = to a flag that takes no args)
# The parser will read 'q', then see '=' as an invalid short option.
run_test "Quiet Garbage (-q=)" "-q= 8.8.8.8" 64 "ping: invalid option -- '='" 1 "" 0

finish_tests "Quiet Bonus"
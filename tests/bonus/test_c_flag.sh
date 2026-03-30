#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "  Running ft_ping Count Tests  "
echo "========================================"

require_root "Bonus -c Flag"

# ---------------------------------------------------------
# Happy Paths: Standard Limits
# (Since the main loop is fixed, these should now finish 
# successfully and exit cleanly with code 0)
# ---------------------------------------------------------
run_test "Short Flag -c 3" "-c 3 127.0.0.1" 0 "PING 127.0.0.1"
run_test "Concatenated Short Flag -c3" "-c3 127.0.0.1" 0 "PING 127.0.0.1"
run_test "Long Flag --count=3" "--count=3 127.0.0.1" 0 "PING 127.0.0.1"

# ---------------------------------------------------------
# inetutils 2.0 Quirks: Infinite Loops
# (These bypass the limit logic and run infinitely, so we 
# rely on the test helper's 2-second timeout -> exit 124)
# ---------------------------------------------------------
run_test "Zero value -c 0 (Bypasses check)" "-c 0 127.0.0.1" 124 "PING 127.0.0.1"
run_test "Negative value -c -5 (Underflows to max)" "-c -5 127.0.0.1" 124 "PING 127.0.0.1"
run_test "Overflow value (Caps at ULONG_MAX)" "-c 99999999999999999999 127.0.0.1" 124 "PING 127.0.0.1"

# ---------------------------------------------------------
# Sad Paths: True Parsing Errors
# (These should fail instantly with EX_USAGE / 64)
# ---------------------------------------------------------
run_test "Missing argument -c" "-c" 64 "option requires an argument -- 'c'"
run_test "Missing argument --count" "--count" 64 "option '--count' requires an argument"
run_test "Garbage string -c abc" "-c abc 127.0.0.1" 64 "invalid value (\`abc' near \`abc')"
run_test "Trailing garbage -c 5abc" "-c 5abc 127.0.0.1" 64 "invalid value (\`5abc' near \`abc')"

finish_tests "Bonus -c Flag"

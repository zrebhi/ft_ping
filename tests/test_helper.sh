#!/bin/bash

# --- Shared Variables ---
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'
PING_BIN="./ft_ping"
FAILS=0

# --- Binary Check ---
if [ ! -f "$PING_BIN" ]; then
    echo -e "${RED}Error: $PING_BIN not found. Run 'make' first.${NC}"
    exit 1
fi

run_test() {
    local test_name="$1"
    local args="$2"
    local expected_code=$3
    local expected_str="$4"
    local duration=${5:-2} # Default timeout of 2 seconds
    local sig_flag=${6:-}  # Optional 6th argument for specific signals
    local is_regex=${7:-0} # Optional 7th arg (1 = Regex Match, 0 = Literal String)

    # Print the header and the exact command being run
    echo -e "${BLUE}▶ [TEST]${NC} ${test_name}"
    echo -e "  ${YELLOW}Cmd:${NC} sudo $PING_BIN $args"

    # Run the binary and capture both stdout and stderr
    local output
    output=$(timeout $sig_flag $duration $PING_BIN $args 2>&1)
    local exit_code=$?

    # Check if the exit code matches
    if [[ $exit_code -eq $expected_code ]] || [[ $exit_code -eq 124 && $expected_code -eq 0 ]]; then
        
        # Branch logic based on the Regex flag
        if [[ "$is_regex" -eq 1 ]]; then
            if echo "$output" | grep -E -q "$expected_str"; then
                echo -e "  ${CYAN}Regex Matched:${NC} $expected_str"
                echo -e "  ${GREEN}✔ PASS${NC}\n"
                return 0
            fi
        else
            if [[ "$output" == *"$expected_str"* ]]; then
                echo -e "  ${CYAN}Output Matched:${NC}"
                echo "$output" | sed 's/^/    /' | head -n 3
                echo -e "  ${GREEN}✔ PASS${NC}\n"
                return 0
            fi
        fi
    fi

    # Failure Output
    echo -e "${RED}[FAIL]${NC} $test_name"
    echo "       Expected Exit Code: $expected_code, Got: $exit_code"
    echo "       Expected Match: '$expected_str'"
    echo "       Actual Output snippet: $(echo "$output" | head -n 1)"
    FAILS=$((FAILS+1))
}

finish_tests() {
    local suite_name="$1"
    echo "========================================"
    if [ $FAILS -eq 0 ]; then
        echo -e "${GREEN}All $suite_name tests passed successfully!${NC}"
        exit 0
    else
        echo -e "${RED}$FAILS $suite_name test(s) failed.${NC}"
        exit 1
    fi
}

require_root() {
    local suite_name="$1"
    if [ "$EUID" -ne 0 ]; then
        echo -e "${YELLOW}[SKIP]${NC} $suite_name tests require root privileges."
        echo "       Tip: Run 'sudo make test' to execute this suite."
        exit 0 # Exit cleanly so the Makefile continues to the next script
    fi
}

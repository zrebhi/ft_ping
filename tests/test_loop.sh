#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Loop Tests        "
echo "========================================"

require_root "Loop"

echo -e "${BLUE}▶ [TEST]${NC} 3-Second Execution Loop (google.com)"
echo -e "  ${YELLOW}Cmd:${NC} timeout 3.5 sudo $PING_BIN google.com"

# Run the binary with a 3.5-second timeout to allow 3 full seconds (and 3 packets) to process
output=$(timeout 3.5 sudo $PING_BIN google.com 2>&1)
exit_code=$?

# timeout returns 124 when the command successfully times out
if [ $exit_code -eq 124 ]; then
    # Count occurrences of our valid reply string
    packet_count=$(echo "$output" | grep -c "bytes from")
    
    if [ "$packet_count" -ge 3 ] && [ "$packet_count" -le 4 ]; then
        echo -e "  ${CYAN}Output:${NC}"
        echo "$output" | sed 's/^/    /'
        echo -e "  ${GREEN}✔ PASS (Sent $packet_count packets before timeout)${NC}\n"
    else
        echo -e "${RED}[FAIL]${NC} Expected ~3 packets, got $packet_count"
        FAILS=$((FAILS+1))
    fi
else
    echo -e "${RED}[FAIL]${NC} Process did not time out correctly (Exit Code: $exit_code)"
    FAILS=$((FAILS+1))
fi

finish_tests "Loop"

#!/bin/bash

source ./tests/test_helper.sh

echo "========================================"
echo "      Running ft_ping Verbose Tests     "
echo "========================================"

require_root "Verbose"

run_verbose_diff_test() {
    # Target a reliable external host. The specific router responding to TTL=1 will be masked later.
    local target="8.8.8.8"

    echo -e "${BLUE}▶ [TEST]${NC} Verbose Dump Diff vs inetutils-2.0"
    
    # Execute system ping and ft_ping, forcing TTL=1 to trigger an ICMP Time Exceeded response
    local sys_raw
    sys_raw=$(ping -v --ttl=1 -c 1 $target 2>&1)

    local ft_raw
    ft_raw=$(timeout 2 sudo env FT_PING_FORCE_TTL=1 $PING_BIN -v $target 2>&1)

    # Sanitize output to ensure stable diffing across different environments and network routes
    sanitize() {
        echo "$1" | grep -A 4 "IP Hdr Dump:" | head -n 5 | \
        
        # Normalize the router response line to prevent failures due to different network gateways
        sed -E 's/^[0-9]+ bytes from [^:]+: Time to live exceeded/XX bytes from ROUTER: Time to live exceeded/' | \
        
        # Mask the raw 16-bit hex dump as it contains dynamic IDs and checksums
        sed -E '/^ [0-9a-f]{4} / s/[0-9a-f]{4}/XXXX/g' | \

        # Selectively mask volatile IP header fields (ID and Checksum) while preserving structural fields 
        # (Version, Header Length, Protocol, etc.) to verify accurate C parsing logic.
        sed -E 's/^( [0-9a-f]+  [0-9a-f]+  [0-9a-f]{2} [0-9a-f]{4} )[0-9a-f]{4}(   [0-9a-f]+ [0-9a-f]{4}  [0-9a-f]{2}  [0-9a-f]{2} )[0-9a-f]{4}( .*)$/\1XXXX\2XXXX\3/' | \
        
        # Mask the nested ICMP sequence and ID fields
        sed -E 's/id 0x[0-9a-f]+, seq 0x[0-9a-f]+/id 0xXXXX, seq 0xXXXX/'
    }

    local sys_clean=$(sanitize "$sys_raw")
    local ft_clean=$(sanitize "$ft_raw")

    echo "$sys_clean" > /tmp/sys_dump.txt
    echo "$ft_clean" > /tmp/ft_dump.txt

    local diff_output
    diff_output=$(diff -u /tmp/sys_dump.txt /tmp/ft_dump.txt)

    if [ -z "$diff_output" ] && [ -n "$ft_clean" ]; then
        echo -e "  ${CYAN}Diff perfectly matched inetutils-2.0${NC}"
        echo -e "  ${GREEN}✔ PASS${NC}\n"
    else
        echo -e "${RED}[FAIL]${NC} Verbose formatting mismatch"
        echo -e "  ${YELLOW}Diff Output (-sys +ft):${NC}"
        echo "$diff_output" | sed 's/^/    /'
        FAILS=$((FAILS+1))
    fi
    
    rm -f /tmp/sys_dump.txt /tmp/ft_dump.txt
}

run_verbose_diff_test
finish_tests "Verbose"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

// 1. FIXED: Changed 'char subnet' to 'const char *subnet'
void ping_sweep(const char *subnet) {
    char cmd[128], ip[32];

    printf("[*] Starting ping sweep on subnet %s.0/24\n", subnet);

    for (int i = 1; i <= 254; i++) {
        sprintf(ip, "%s.%d", subnet, i);
        // -n 1 sends one packet, -w 100 waits 100ms for response
        sprintf(cmd, "ping -n 1 -w 100 %s > nul", ip);
        system(cmd);
        printf("Pinging %s...\r", ip); // Progress indicator
    }
    printf("\n[+] Ping sweep finished.\n");
}

// 2. FIXED: Changed 'HostTable table' to 'HostTable *table' and 'FILEfp' to 'FILE *fp'
void read_arp_table(HostTable *table) {
    FILE *fp = _popen("arp -a", "r");
    if (!fp) {
        printf("[-] Failed to read ARP table\n");
        return;
    }

    char line[256], ip[32];

    while (fgets(line, sizeof(line), fp)) {
        // Parse IP address at start of line
        // We look for patterns like 192.168.1.1
        if (sscanf(line, " %31[0-9.]", ip) == 1) {
            // Check if it's a valid IP length to avoid headers
            if (strlen(ip) > 6) {
                table_add_host(table, ip);
            }
        }
    }

    _pclose(fp);
}

// 3. FIXED: Changed 'char ip' to 'const char *ip' and fixed struct casting
int scan_port(const char *ip, int port) {
    SOCKET sock;
    struct sockaddr_in addr;
    DWORD timeout = 300; // 300ms timeout

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return 0;

    // FIXED: Added (const char*)& for the timeout pointers
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    // FIXED: Added (struct sockaddr *)& for proper casting
    int result = connect(sock, (struct sockaddr *)&addr, sizeof(addr));

    closesocket(sock);

    // If result is 0, connection was successful (Port is open)
    return result == 0;
}

void get_local_subnet(char *subnet_out) {
    char hostname[256];
    struct hostent *host_entry;
    struct in_addr **addr_list;

    // Default fallback if detection fails
    strcpy(subnet_out, "192.168.1");

    // 1. Get the name of this computer
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) return;

    // 2. Get the IP addresses associated with this name
    host_entry = gethostbyname(hostname);
    if (host_entry == NULL) return;

    addr_list = (struct in_addr **)host_entry->h_addr_list;

    for (int i = 0; addr_list[i] != NULL; i++) {
        char *full_ip = inet_ntoa(*addr_list[i]);

        // Skip loopback addresses (127.0.0.1)
        if (strcmp(full_ip, "127.0.0.1") == 0) continue;

        // 3. Logic to strip the last octet (e.g., 192.168.1.45 -> 192.168.1)
        strcpy(subnet_out, full_ip);
        char *last_dot = strrchr(subnet_out, '.');
        if (last_dot) {
            *last_dot = '\0'; // Cut the string at the last dot
        }
        return; // Return the first valid physical IP found
    }
}

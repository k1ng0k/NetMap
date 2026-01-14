#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>  // Added
#include "network.h"   // Added
#include "table.h"     // Added

// Link the Windows Socket library
#pragma comment(lib, "ws2_32.lib")

void banner() {
    printf("\x1b[33m"); // Yellow banner
    printf("=====================================\n");
    printf("   NETMAP   \n");
    printf("=====================================\n");
    printf("\x1b[0m");
}

void help() {
    printf("\nAbout:\n");
    printf("This is a simple network mapper inspired by nmap.\n");
    printf("It discovers hosts using ping + ARP and scans ports.\n");
    printf("Designed for educational use only.\n\n");
}

int main() {
    system("chcp 65001 > nul");
    WSADATA wsa;
    HostTable table;
    int choice;
    char subnet[32];
    int ports[] = {22, 80, 443, 445, 3389};
    int port_count = 5;

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("[-] Winsock init failed.\n");
        return 1;
    }

    // Initialize the table and detect the network automatically
    table_init(&table);
    get_local_subnet(subnet);

    banner();
    printf("[*] Auto-detected Subnet: %s.0/24\n", subnet);

    while (1) {
        printf("\nMenu: [1] Scan [2] Table [3] Topology [4] Save [5] Help [0] Exit\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 0) break;

        if (choice == 1) {
            // Reset table for fresh scan
            table_free(&table);
            table_init(&table);

            printf("\n[+] Scanning %s.1 to %s.254...\n", subnet, subnet);
            ping_sweep(subnet);

            printf("[+] Gathering device info (ARP)...\n");
            read_arp_table(&table);

            printf("[+] Checking ports on %d hosts...\n", table.count);
            for (int i = 0; i < table.count; i++) {
                Host *h = &table.hosts[i];
                h->port_count = 0; // Reset port count to avoid overflow

                for (int p = 0; p < port_count; p++) {
                    printf("    Scanning %s:%d \r", h->ip, ports[p]);
                    if (scan_port(h->ip, ports[p])) {
                        if (h->port_count < MAX_PORTS) {
                            h->open_ports[h->port_count++] = ports[p];
                        }
                    }
                }
            }
            printf("\n[+] Done.\n");
        }
        else if (choice == 2) table_print(&table);
        else if (choice == 3) table_print_topology(&table, subnet);
        else if (choice == 4) table_save_topology(&table, subnet, "topology.txt");
        else if (choice == 5) {
            printf("\nAuto-Detect Info:\nYour IP belongs to the %s.x range.\n", subnet);
        }
    }

    table_free(&table);
    WSACleanup();
    return 0;
}

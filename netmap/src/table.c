#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ANSI color codes
#define GREEN "\x1b[32m"
#define BLUE  "\x1b[34m"
#define CYAN  "\x1b[36m"
#define RESET "\x1b[0m"

void table_init(HostTable *table) {
    table->count = 0;
    table->capacity = 8;
    table->hosts = malloc(sizeof(Host) * table->capacity);

    // Safety check: Always ensure malloc worked
    if (!table->hosts) {
        printf("[-] Memory allocation failed!\n");
        exit(1);
    }
}

Host* table_get_host(HostTable *table, const char *ip) {
    for (int i = 0; i < table->count; i++)
        if (strcmp(table->hosts[i].ip, ip) == 0)
            return &table->hosts[i];
    return NULL;
}

void table_add_host(HostTable *table, const char *ip) {
    if (table_get_host(table, ip)) return; // Don't add same IP twice

    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->hosts = realloc(table->hosts, sizeof(Host) * table->capacity);
    }

    Host *h = &table->hosts[table->count++];
    strncpy(h->ip, ip, 31); // Safer than strcpy
    h->port_count = 0;
    strcpy(h->type, "Unknown");
}

// Simple heuristic for device type
void table_detect_type(Host *h) {
    // If Port 80 or 443 is open, it's likely a Web Server
    int has_web = 0;
    for(int i=0; i < h->port_count; i++) {
        if(h->open_ports[i] == 80 || h->open_ports[i] == 443) has_web = 1;
    }

    if (h->port_count == 1 && h->open_ports[0] == 22) {
        strcpy(h->type, "Linux/SSH Device");
    } else if (has_web) {
        strcpy(h->type, "Web Server/Router");
    } else if (h->port_count >= 1) {
        strcpy(h->type, "Personal PC");
    } else {
        strcpy(h->type, "Active Host");
    }
}

void table_print(const HostTable *table) {
    printf("\n--- Scanning Results ---\n");
    for (int i = 0; i < table->count; i++) {
        // Use a pointer so we update the ACTUAL host data
        Host *h = &table->hosts[i];
        table_detect_type(h);

        const char *color = (strcmp(h->type, "Personal PC") != 0) ? BLUE : CYAN;
        printf("%sHost: %-15s [%s]%s\n", color, h->ip, h->type, RESET);

        for (int p = 0; p < h->port_count; p++)
            printf("  " GREEN "[+] Port %d is OPEN\n" RESET, h->open_ports[p]);
    }
}

void table_print_topology(const HostTable *table, const char *subnet) {
    printf("\n" BLUE "Network Topology View" RESET "\n");
    printf("=====================\n");
    printf("%s.0/24 (Subnet)\n", subnet);

    for (int i = 0; i < table->count; i++) {
        Host *h = &table->hosts[i];
        table_detect_type(h);

        const char *color = (h->port_count > 0) ? GREEN : RESET;

        // Visual tree structure
        if (i == table->count - 1)
            printf("└── %s%s%s [%s]\n", color, h->ip, RESET, h->type);
        else
            printf("├── %s%s%s [%s]\n", color, h->ip, RESET, h->type);

        for (int p = 0; p < h->port_count; p++) {
            printf("│   └── Port %d\n", h->open_ports[p]);
        }
    }
}

void table_save_topology(const HostTable *table, const char *subnet, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("[-] Could not save file!\n");
        return;
    }

    fprintf(f, "Network Topology Log\n====================\n\n");
    fprintf(f, "Subnet: %s.0/24\n\n", subnet);

    for (int i = 0; i < table->count; i++) {
        Host *h = &table->hosts[i];
        table_detect_type(h);
        fprintf(f, "[+] Host: %s (%s)\n", h->ip, h->type);
        for (int p = 0; p < h->port_count; p++)
            fprintf(f, "    |-- Port: %d (OPEN)\n", h->open_ports[p]);
        fprintf(f, "\n");
    }

    fclose(f);
}

void table_free(HostTable *table) {
    if (table->hosts) {
        free(table->hosts);
        table->hosts = NULL;
    }
    table->count = 0;
}

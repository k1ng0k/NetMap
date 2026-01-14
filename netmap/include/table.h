#ifndef TABLE_H
#define TABLE_H

#define MAX_PORTS 16

typedef struct {
    char ip[32];
    int open_ports[MAX_PORTS];
    int port_count;
    char type[32];  // e.g., "PC", "Server", "Printer"
} Host;

typedef struct {
    Host *hosts;    // Fixed: Now a pointer for an array of hosts
    int count;      // Current number of hosts found
    int capacity;   // Maximum space allocated
} HostTable;

// --- Function Prototypes ---

// Use pointers (*) so the functions can actually modify the table
void table_init(HostTable *table);
void table_add_host(HostTable *table, const char *ip);
Host* table_get_host(HostTable *table, const char *ip);
void table_detect_type(Host *h);

// Use 'const' for printing because we are only reading, not changing
void table_print(const HostTable *table);
void table_print_topology(const HostTable *table, const char *subnet);
void table_save_topology(const HostTable *table, const char *subnet, const char *filename);

// Free the memory when done
void table_free(HostTable *table);

#endif

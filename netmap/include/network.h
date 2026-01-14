#ifndef NETWORK_H
#define NETWORK_H

#include "table.h"

// 1. Changed to 'const char *' to accept strings like "192.168.1"
void ping_sweep(const char *subnet);

// 2. Added space and '*' to pass the table by reference
void read_arp_table(HostTable *table);

// 3. Ensure this matches the implementation in network.c
int scan_port(const char *ip, int port);

void get_local_subnet(char *subnet_out);

#endif

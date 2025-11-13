#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>

typedef struct Device {
    char id[64];
    char ip[32];
    char mac[32];
    char type[16];
    char vendor[64];
    struct Device *next;
} Device;

typedef struct {
    Device *head;
    int count;
} DeviceList;

DeviceList* createDeviceList(void);
void add_dev(DeviceList *list, const char *id, const char *ip,const char *mac, const char *type, const char *vendor);
Device* find_dev(DeviceList *list, const char *ip);
void remove_dev(DeviceList *list, const char *ip);
void free_devlist(DeviceList *list);


int initNetworkScan(const char *interface);
int scanNetwork(DeviceList *list, const char *subnet);
int pingDevice(const char *ip);
int getMACAddress(const char *ip, char *macBuffer, size_t bufferSize);
int querySNMP(const char *ip, char *vendorBuffer, size_t bufferSize);
int discoverNeighbors(DeviceList *list, const char *ip);
void shutdownNetworkScan(void);

// === Import/Export ===
//int saveNetworkToFile(const DeviceList *list, const char *filename);
//int loadNetworkFromFile(DeviceList *list, const char *filename);

#endif
#ifndef _NETBIOS_H
#define _NETBIOS_H

#define ALIAS_SIZE 32
#define DSP_SIZE 64
#include <arpa/inet.h>

#include "list.h"

struct host {
	struct in_addr ipaddr;
	unsigned char mac[6];
	char alias[ALIAS_SIZE];
	char description[DSP_SIZE];
	struct list_head list;
};

void start_network_probe(struct in_addr *ipaddr, 
	struct in_addr *netmask, struct list_head *list);

struct host *find_host_by_ipaddr(struct in_addr ipaddr,
	struct list_head *head);

#endif

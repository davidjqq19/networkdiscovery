#ifndef _IFACE_H
#define _IFACE_H

#include <arpa/inet.h>
#include "list.h"

#define IFACE_SIZE 32

struct if_info {
	char iface[IFACE_SIZE];		//interface
	unsigned char mac[6];		//mac address
	struct in_addr ipaddr;		//ip address
	struct in_addr netmask;		//netmask
	struct list_head list;
};

int get_if_info(struct list_head *list);

#endif

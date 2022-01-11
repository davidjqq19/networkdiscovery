#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include "iface.h"

int get_local_mac(unsigned char *mac, int len_limit,char *iface);

/*only ipv4*/
int get_if_info(struct list_head *list)
{
	struct ifaddrs *ifa = NULL;
	struct if_info *tmp;

	if(getifaddrs(&ifa) < 0)
	{
		perror("getifaddrs");
		return -1;
	}
	
	for(; ifa != NULL; ifa = ifa->ifa_next)
	{
		if ((ifa->ifa_addr)->sa_family != AF_INET)
			continue;
		tmp = (struct if_info *)malloc(sizeof(struct if_info));
		if(tmp == NULL)
		{
			perror("malloc");
			return -1;
		}

		strcpy(tmp->iface, ifa->ifa_name);
		if(get_local_mac(tmp->mac, 6, ifa->ifa_name) < 0)
		{
			perror("get local mac");
			return -1;
		}
		tmp->ipaddr = ((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr;
		tmp->netmask = ((struct sockaddr_in *)(ifa->ifa_netmask))->sin_addr;
		list_add(&(tmp->list), list);
	}
	return 1;
}

int get_local_mac(unsigned char *mac, int len_limit,char *iface)
{
    struct ifreq ireq;
    int sock;
	int i;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        return -1;
    }
    strcpy (ireq.ifr_name, iface);

    if (ioctl (sock, SIOCGIFHWADDR, &ireq) < 0)
    {
        perror ("ioctl");
        return -1;
    }
   
	for(i=0; i<len_limit; i++)
	{
		mac[i] = ireq.ifr_hwaddr.sa_data[i]; 
	}
	return 1;
}

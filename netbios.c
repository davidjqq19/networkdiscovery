#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>

#include "netbios.h"
#include "iface.h"

#define NETBIOS_PORT 137
#define MAX_THREAD 256
#define TIMEOUTVAL 1
#define UTIMEOUTVAL 0
#define MAX_BUFF_SIZE 1024

static unsigned char netbios_request[] = {
	0x82, 0x28, 0x0, 0x0, 0x0,
    0x1, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x20, 0x43, 0x4B,
    0x41, 0x41, 0x41, 0x41, 0x41,
    0x41, 0x41, 0x41, 0x41, 0x41,
    0x41, 0x41, 0x41, 0x41, 0x41,
    0x41, 0x41, 0x41, 0x41, 0x41,
    0x41, 0x41, 0x41, 0x41, 0x41,
    0x41, 0x41, 0x41, 0x41, 0x41,
    0x0, 0x0, 0x21, 0x0, 0x1
};

void *get_target_netbios(void *arg)
{
	int skfd;
	int retval;
	socklen_t sockaddr_len;
	struct timeval timeout = {TIMEOUTVAL, UTIMEOUTVAL};
	char buff[MAX_BUFF_SIZE];
	struct in_addr target = *(struct in_addr *)arg;
	struct host *hostp = NULL;
	
	struct sockaddr_in dstaddr;
	memset(&buff, 0, MAX_BUFF_SIZE);
	memset(&dstaddr, 0, sizeof(struct sockaddr_in));
	dstaddr.sin_family = AF_INET;
	dstaddr.sin_addr = target;
	dstaddr.sin_port = htons(NETBIOS_PORT);
	sockaddr_len = (socklen_t)sizeof(struct sockaddr_in);

	skfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(skfd < 0)
	{
		printf("create socket failed,target ip %s.\n", inet_ntoa(target));
		perror("because");
		close(skfd);
		return NULL;
	}
	//printf("create socket success,target ip %s\n", inet_ntoa(target));
	retval = setsockopt(skfd, SOL_SOCKET, SO_RCVTIMEO,
		(char *)&timeout, sizeof(struct timeval));
	if(retval < 0)
	{
		perror("setsockopt");
	}

	retval = sendto(skfd, netbios_request, sizeof(netbios_request), 0,
		(struct sockaddr *)&dstaddr, sockaddr_len);
	if(retval < 0)	
	{
		printf("send udp packet failed,target ip %s.\n", inet_ntoa(target));
		perror("because");
		close(skfd);
		return NULL;
	}

	retval = recvfrom(skfd, buff, MAX_BUFF_SIZE, MSG_WAITALL,
		(struct sockaddr *)&dstaddr, &sockaddr_len);
	if(retval > 0)
	{
		hostp = (struct host *)malloc(sizeof(struct host));
		if(hostp == NULL)
		{
			perror("malloc struct host failed");
			close(skfd);
			return NULL;
		}
		memset(hostp, 0, sizeof(struct host));
		hostp->ipaddr = target;
		strncpy(hostp->alias, buff+57, 16);
		//printf("host %s,alias:%s\n", inet_ntoa(hostp->ipaddr), hostp->alias);
	}
	else if(retval == -1)
	{
		if(errno != 11)
			perror("recvfrom");
	}
	
	close(skfd);
	return (void *)hostp;	
}

int get_nmask(struct in_addr mask)
{
	int i;
	in_addr_t m = htonl(mask.s_addr);
	for(i=0; m<<=1; i++);
	if(i) i++;
	return i;
} 

void join_the_threads(pthread_t *pids, int len, struct list_head *list)
{
	int i=0;
	int retval;
	struct host *hostp=NULL;
	for(;i < len; i++)
	{
		if(pids[i] == 0)
			continue;
		retval = pthread_join(pids[i], (void **)&hostp);
		if(retval < 0)
		{
			perror("pthread join failed");
		}
		
		if(hostp)
		{
			list_add(&(hostp->list), list);
		}
	}
	//printf("join over.\n");
}

void clear_targets(struct in_addr *t, int len)
{
	int i;
	for(i=0; i<len; i++)
		t[i] = (struct in_addr){(unsigned int)0};
	return;
}

void clear_pids(pthread_t *t, int len) 
{
	int i;
	for(i=0; i<len; i++)
		t[i] = (pthread_t)0;
	return;
}

void start_network_probe(struct in_addr *ipaddr, 
	struct in_addr *netmask, struct list_head *list)
{
	int i, retval;
	unsigned int network;
	pthread_t pids[MAX_THREAD] = {0};
	struct in_addr targets[MAX_THREAD] = {{(unsigned int)0}};

	int nmask = get_nmask(*netmask);	//bits of netmask
	int ipmax = pow(2, (32 - nmask)) - 2;	//Maximum number of ip address
	network = ntohl(ipaddr->s_addr) & 
		ntohl(netmask->s_addr);
	//printf("%d,%d\n",nmask,ipmax);
	//printf("network:%x\n",network);

	for(i=1;i<=ipmax;i++)
	{
		targets[i%MAX_THREAD].s_addr = htonl(network + i);
		retval = pthread_create(
			&(pids[i%MAX_THREAD]), 
			NULL, 
			get_target_netbios, 
			&targets[i%MAX_THREAD]);
		//printf("create pthread,target ip %s\n",inet_ntoa(targets[i%MAX_THREAD]));
		if(retval)
		{
			printf("create pthread failed,target ip %s,",inet_ntoa(targets[i%MAX_THREAD]));
			perror("because");
		}
		
		if((!(i%MAX_THREAD)) || (i == ipmax))
		{
			join_the_threads(pids, MAX_THREAD, list);
			clear_pids(pids, MAX_THREAD);
			clear_targets(targets, MAX_THREAD);
		}
	}

	//printf("NetBios over.\n");
}

struct host *find_host_by_ipaddr(struct in_addr ipaddr, struct list_head *head)
{
	struct host *pos;
	list_for_each_entry(pos, head, list)
	{
		if(pos->ipaddr.s_addr == ipaddr.s_addr)
			return pos;
	}
	
	return NULL;
}

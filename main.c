#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iface.h"
#include "netbios.h"
#include "arpreader.h"

#define DEFAULT_ETH_NAME "eth0"
#define MAC_PREFIX_PATH "nmap-mac-prefixes"
#define ARP_FILE_PATH "/proc/net/arp"
#define PREFIX_FILE_SIZE 409600
#define ARP_FILE_SIZE 256
#define ETH_NAME_SIZE 128

void host_list_destory(struct list_head *head);
void iface_list_destory(struct list_head *head);

int main(int argc, char **argv)
{
	LIST_HEAD(if_list);
	LIST_HEAD(host_list);

	struct if_info *ipos, *use = NULL;
	struct host *hpos;
	FILE *prefix_file, *arp_file;
	char prefix_buff[PREFIX_FILE_SIZE];
	char arp_buff[ARP_FILE_SIZE];
	char eth_name[ETH_NAME_SIZE] = DEFAULT_ETH_NAME;

	struct in_addr arp_addr;
	char arp_mac[7] = {0};
	char mac_prefix[7] = {0};
	int retval;
	struct host *target;

	int oc;
	while((oc = getopt(argc, argv, "i:")) != -1)
	{
		switch(oc)
		{
			case 'i':
				if(strlen(optarg) < ETH_NAME_SIZE - 1)
				{
					memset(eth_name, 0, ETH_NAME_SIZE);
					strcpy(eth_name, optarg);
				}
				else
				{
					printf("interface name is too long.\n");
					exit(-1);
				}
				break;
		}
	}


	prefix_file = fopen(MAC_PREFIX_PATH, "r");
	if(prefix_file == NULL)
	{
		perror("open mac prefix file failed");
		exit(-1);
	}
	arp_file = fopen(ARP_FILE_PATH, "r");
	if(arp_file == NULL)
	{
		perror("open arp file failed");
		exit(-1);
	}


	/*读取本地网卡信息*/
	if(get_if_info(&if_list) < 0)
	{
		exit(-1);
	}

	/*打印本地网卡信息*/
	printf("Local interfaces:\n");
	list_for_each_entry(ipos, &if_list, list)
	{
		char ip_tmp[32];
		char mask_tmp[32];
		strcpy(ip_tmp, inet_ntoa(ipos->ipaddr));
		strcpy(mask_tmp, inet_ntoa(ipos->netmask));
		printf("iface:%s\tinet:%s\tnetmask:%s\tmac:%02X:%02X:%02X:%02X:%02X:%02X\n", 
			ipos->iface, ip_tmp, mask_tmp,  
			ipos->mac[0],ipos->mac[1],ipos->mac[2],
			ipos->mac[3],ipos->mac[4],ipos->mac[5]);
		if(strcmp((ipos->iface), eth_name) == 0)
		{
			use = ipos;
		}
	}

	if(use == NULL)
	{
		printf("No interface named \"%s\"\n", eth_name);
		exit(-1);
	}

	struct in_addr mask;
	mask.s_addr = inet_addr("255.255.255.0");
	/*NetBios网络探测,获取主机名，结果存入host_list表*/
	start_network_probe(&(use->ipaddr), &mask, &host_list);

	/*读取系统arp表，将主机mac地址添加到host_list表*/
	while(fgets(arp_buff, ARP_FILE_SIZE, arp_file))
	{
		retval = resolve_mac_address(arp_buff, &arp_addr, arp_mac);
		if(retval > 0)
		{
			target = find_host_by_ipaddr(arp_addr, &host_list);
			if(target == NULL)
			{
				struct host *newhost = (struct host *)malloc(sizeof(struct host));
				if(newhost == NULL)
				{
					perror("malloc struct host");
					return -1;
				}
				memset(newhost, 0, sizeof(struct host));
				newhost->ipaddr = arp_addr;
				memcpy(newhost->mac, arp_mac, 6);
				list_add(&(newhost->list), &host_list);

			}
			else
			{
				memcpy(target->mac, arp_mac, 6);
			}
		}
	}

	/*根据网卡Mac地址匹配对应的厂商信息*/
	fread(prefix_buff, PREFIX_FILE_SIZE, 1, prefix_file);
	list_for_each_entry(hpos, &host_list, list)
	{
		snprintf(mac_prefix, 7, "%02X%02X%02X",
			hpos->mac[0],hpos->mac[1],hpos->mac[2]);
		//printf("prefix:%s\n", mac_prefix);
		get_mac_description_from_prefix(mac_prefix, prefix_buff, hpos->description);
	}
		
	/*遍历并打印信息*/
	printf("Hosts:\n");
	list_for_each_entry(hpos, &host_list, list)
	{
		printf("inet:%s\tmac:%02X:%02X:%02X:%02X:%02X:%02X\talias:%s\tdes:%s\n", 
			inet_ntoa(hpos->ipaddr), 
			hpos->mac[0],hpos->mac[1],hpos->mac[2],
			hpos->mac[3],hpos->mac[4],hpos->mac[5],
			hpos->alias, hpos->description);
	}

	host_list_destory(&host_list);
	iface_list_destory(&if_list);
	return 0;
}

void host_list_destory(struct list_head *head)
{
	struct host *pos, *tmp;
	list_for_each_entry_safe(pos, tmp, head, list)
	{
		list_del_init(&(pos->list));
		free(pos);
	}
}

void iface_list_destory(struct list_head *head)
{
	struct if_info *pos, *tmp;
	list_for_each_entry_safe(pos, tmp, head, list)
	{
		list_del_init(&(pos->list));
		free(pos);
	}
}


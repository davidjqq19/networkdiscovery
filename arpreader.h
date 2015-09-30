#include "list.h"
#include <arpa/inet.h>

void get_mac_description_from_prefix(char *prefix, char *src, char *dst);

int resolve_mac_address(char *src, struct in_addr *addr, char *mac);


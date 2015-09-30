#include <stdio.h>
#include <string.h>
#include <pcre.h> 
#include "arpreader.h"
    
#define OVECCOUNT 30    /* should be a multiple of 3 */
#define EBUFLEN 128            
#define BUFLEN 1024           
#define FILE_BUFF 1024
#define LINE_SIZE 128
#define LINES 10
#define MAX_PATTERN_SIZE 128
    
int mac_str_to_bin( char *str, char *mac)
{
    int i;
    char *s, *e;

    if ((mac == NULL) || (str == NULL))
    {
        return -1;
    }

    s = (char *) str;
    for (i = 0; i < 6; ++i)
    {
        mac[i] = s ? strtoul (s, &e, 16) : 0;
        if (s)
           s = (*e) ? e + 1 : e;
    }
    return 0;
}

    
int resolve_mac_address(char *src, struct in_addr *addr, char *mac) 
{               
	pcre *re; 
	const char *error;
	int erroffset;
	int ovector[OVECCOUNT];
	int rc, i;
	char group[LINES][LINE_SIZE] = {{0}};
        
	char pattern[] = "^([\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3})\\s+([0-9a-fx]+)\\s+([0-9a-fx]+)\\s+([a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2})\\s+([^\\s]+)\\s+(.+)$";
                
	//printf("String : %s\n", src);
	//printf("Pattern: \"%s\"\n", pattern);
        
	re = pcre_compile(pattern, PCRE_CASELESS, &error, &erroffset, NULL);
	if (re == NULL)
	{
		printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
		return -1;
	}

	rc = pcre_exec(re, NULL, src, strlen(src), 0, 0, ovector, OVECCOUNT);
	if (rc < 0)
	{
		if (rc == PCRE_ERROR_NOMATCH) 
			;
		else
			printf("Matching error %d\n", rc);
		free(re);
		return -1;
	}

	for (i = 0; i < rc; i++)
	{
		char *start = src + ovector[2*i];
		int length = ovector[2*i+1] - ovector[2*i];
		strncpy(group[i], start, length);
		//printf("%2d: %s\n", i, group[i]);
	}

	if(!strncmp(group[4], "00:00:00:00:00:00", 18))
		return 0;

	inet_aton(group[1], addr);
	mac_str_to_bin(group[4], mac);
		
	free(re);
	return 1;
}

void get_mac_description_from_prefix(char *prefix, char *src, char *dst)
{
	pcre *re;
	const char *error;
	int erroffset;
	int ovector[OVECCOUNT];
	int rc;
	char pattern[MAX_PATTERN_SIZE] = {0};

	sprintf(pattern, "^%s\\s(.*)$", prefix);
	//printf("pattern is \"%s\"\n", pattern);
	
	re = pcre_compile(pattern, PCRE_MULTILINE, &error, &erroffset, NULL);
	if (re == NULL)
	{
		printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
		return ;
	}

	rc = pcre_exec(re, NULL, src, strlen(src), 0, 0, ovector, OVECCOUNT);
	if (rc < 0)
	{
		if (rc == PCRE_ERROR_NOMATCH) 
			;
		else
			printf("Matching error %d\n", rc);
		free(re);
		return ;
	}

	char *start = src + ovector[2];
	int length = ovector[3] - ovector[2];
	strncpy(dst, start, length);
	//printf("Match: %s\n", dst);
	free(re);
	return ;
}

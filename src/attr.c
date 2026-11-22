#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "attr.h"

char server_salt[33] = {0};
const char* hb_software = "sn 0.1.13";

char server_name[64] = "A Minecraft Server";
char server_motd[64] = "";
int server_port = 25565;

void trim(char* s)
{
    char* p = s;
    int l = strlen(p);

    while(l > 0 && isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}

int config_load(const char* path)
{
	FILE* fp = fopen(path, "r");
	if (fp == NULL) {
		perror("config: fopen");
		return -1;
	}

	/* PARSE HERE */
	char line[128];
	char k[32], v[64];

	while (fgets(line, sizeof(line), fp) != NULL) {
		line[strcspn(line, "\r\n")] = '\0';

		if (sscanf(line, "%31[^=]=%63[^\n]", k, v) == 2) {
			trim(k);
			trim(v);

			printf("DEBUG: Config: scanned\n  trimmed key:%s\n  trimmed value:%s\n", k, v);

			if (strcmp("server-name", k) == 0) {
				snprintf(server_name, sizeof(server_name), "%s", v);
			} else if (strcmp("server-motd", k) == 0) {
				snprintf(server_motd, sizeof(server_motd), "%s", v);
			} else if (strcmp("port", k) == 0) {
				char* end;
				long temp;
				// server_port = strtol(v);
				temp = strtol(v, &end, 10);
				
				if (*end != '\0') {
					printf("ERROR: Config: port must be a number\n");
					continue;
				} else if (/*end != v && *end == '\0' &&*/ temp >= 1 && temp <= 65535) {
					server_port = temp;
				} else {
					printf("ERROR: Config: port out of valid range! (1-65535)\n");
					continue;
				}
			}
		}
	}

	fclose(fp);
	return 0;
}

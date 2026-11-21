#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "attr.h"

char server_salt[33] = {0};
const char* hb_software = "sn 0.1.12-hotfix1";

char server_name[64] = "A Minecraft Server";
char server_motd[64] = "";
int server_port = 25565;

void trim(char* s)
{
    char* p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
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

		if (sscanf("%31[^=]=%63[^\n]") == 2) {
			trim(k);
			trim(v);
			if (strcmp("server-name", k) == 0) {
				snprintf(server_name, sizeof(server_name), "%s", v);
			} else if (strcmp("server-motd", k) == 0) {
				snprintf(server_motd, sizeof(server_motd), "%s", v);
			} else if (strcmp("port", k) == 0) {
				server_port = strtol(v);
			}
		}
	}

	fclose(fp);
	return 0;
}

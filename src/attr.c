#include <stdio.h>
#include <string.h>

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

char* config_get(char* buffer, const char* key)
{
	/*open FILE* fp
	  search for const char* key
	  look for '=', find value after '='
	  separate key and value
	  return value
	  (copy to buffer as func. argument?
	  get size of buffer as arg.?)*/
	char* value;
	sscanf(buffer, "%32s[^=]=%s", key, &value);
	return value;
}

int config_get_int(char* buffer, const char* key)
{
	/*open FILE* fp
	  search for const char* key
	  look for '=', find value after '='
	  separate key and value
	  change string value into int valua
	  return value*/
	int value;
	sscanf(buffer, "%32s[^=]=%d", key, &value);
	return value;
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
		}
		/* PARSE HERE
		 * we already have one line to parse on the fly HERE. config_get and config_get_line are not needed at all. use something safer than sscanf()! */

		snprintf(server_name, sizeof(server_name), "%s", /*value*/ config_get(line, "server-name"));
		snprintf(server_motd, sizeof(server_name), "%s", /*value*/ config_get(line, "motd"));

		server_port = config_get_int(line, "port");
	}

	fclose(fp);
	return 0;
}

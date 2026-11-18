#include <stdio.h>

#include "attr.h"

char server_salt[33] = {0};
const char* hb_software = "sn 0.1.12-hotfix1";

char server_name[64] = "A Minecraft Server";
char server_motd[64] = "";
int server_port = 25565;


char* trim_whitespace(char* buffer)
{
    for (int i = 0; i < sizeof(buffer), i++) {
        while (buffer[i] == '\t' ||
                buffer[i] == '\n' ||
                buffer[i] == '\r') continue;
    }
    /*while scanned char == '\0' continue
    copy characters to buffer
      until buffer end
    (it's without whitespaces now)
    return buffer*/
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
    sscanf(buffer, "%32[^=]=%s", key, &value);
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
    sscanf(buffer, "%32[^=]=%d", key, &value);
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
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        server_name = config_get(line, "server-name");
        server_motd = config_get(line, "motd");
	server_port = config_get_int(line, "port");
    }

    fclose(fp);
    return 0;
}

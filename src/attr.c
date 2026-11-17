#include <stdio.h>

#include "attr.h"

char server_salt[33] = {0};
const char* hb_software = "sn 0.1.12-hotfix1";

char server_name[64] = "A Minecraft Server";
char server_motd[64] = "";
int port = 25565;

/*
char* trim_whitespace(char* buffer)
{
    while scanned char == '\0' continue
    copy characters to buffer
      until buffer end
    (it's without whitespaces now)
    return buffer
}

char* config_get(FILE* fp, const char* key)
{
    open FILE* fp
    search for const char* key
    look for '=', find value after '='
    separate key and value
    return value
    (copy to buffer as func. argument?
    get size of buffer as arg.?)
}

int config_get_int(FILE* fp, const char* key)
{
    open FILE* fp
    search for const char* key
    look for '=', find value after '='
    separate key and value
    change string value into int valua
    return value
}
*/

int config_load(const char* path)
{
    FILE* fp = fopen(path, "r");
    if (!fp) {
        perror("config: fopen");
	return -1;
    }

    /* PARSE HERE */

    fclose(fp);
    return 0;
}

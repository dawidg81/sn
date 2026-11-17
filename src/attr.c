#include <stdio.h>

#include "attr.h"

char server_salt[33] = {0};
const char* hb_software = "sn 0.1.12-hotfix1";

char server_name[64] = "A Minecraft Server";
char server_motd[64] = "";
int port = 25565;

/*char* trim_whitespace(char* buffer)
{
    while
}

char* config_get(FILE* fp, constbchar* key){}
int config_get_int()
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

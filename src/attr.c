#include <stdio.h>

#include "attr.h"

char server_salt[33] = {0};
const char* hb_software = "sn 0.1.12-hotfix1";

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

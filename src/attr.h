#ifndef ATTR_H
#define ATTR_H

/* unconfigurable constants */
extern char server_salt[33];
extern const char* hb_software; /* = "sn 0.1.12-hotfix1" */

/* attributes to import from the config file
 * every time the server starts */
extern char* server_name;
extern char* server_motd;
extern int port;

int config_load(const char* path);
void config_free();

#endif

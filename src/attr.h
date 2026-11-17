#ifndef ATTR_H
#define ATTR_H

/* unconfigurable constants */
extern char server_salt[33];
extern const char* hb_software; /* = "sn 0.1.12-hotfix1" */

/* attributes to import from the config file
 * every time the server starts */
extern char server_name[64];
extern char server_motd[64];
extern int port;

/* 0 = ok, -1 = file missing */
int config_load(const char* path);

#endif

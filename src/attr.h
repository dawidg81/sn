#ifndef ATTR_H
#define ATTR_H

/* unconfigurable constants */
extern char server_salt[33];
extern const char* hb_software; /* = "sn 0.1.12-hotfix1" */

/* attributes to import from the config file
 * 
 * ? every time the server starts
 * ? OR ?
 * ? every time one of the attributes
 *   need to be used */
extern char* server_name;
extern char* server_motd;
extern int port;

int config_load(const char* path);
void config_free();

#endif

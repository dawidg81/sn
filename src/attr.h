#ifndef ATTR_H
#define ATTR_H

/* unconfigurable constants */
char server_salt[33] = {0};
static const char* hb_software = "sn 0.1.12-hotfix1";

/* attributes to import from the config file
 * 
 * ? every time the server starts
 * ? OR ?
 * ? every time one of the attributes
 *   need to be used */
char* server_name;
char* server_motd;
int port;

#endif

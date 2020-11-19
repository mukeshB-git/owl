#ifndef SERVER_H
#define SERVER_H

#include "esb.h"
#include<stdbool.h>

extern TD process_esb_request(char* bmd_file_path);
extern char * transformjson(char * fname);
extern int  emailsender(char *to,  char *text);
extern int send_ftp_file(char * REMOTE_URL);
extern BMD * parse_bmd_xml(char* bmd_file_path);
int make_named_socket(const char *socket_file, bool is_client);
void log_msg(const char *msg, bool terminate);

#endif
/*
To compile:
    gcc   server.c esb.c ../adapter/email.c ../adapter/xmltojson.c ftpp.c -o ipc -Wall $(xml2-config --cflags) 
    $(xml2-config --libs) -lxml2  $(mysql_config --cflags) $(mysql_config --libs) -lpthread -ljson-c -lcurl

All in a single line

to see output: ./ipc

*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/un.h>
#include <stddef.h>
#include "esb.h"
#include<stdlib.h>
#include<mysql/mysql.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "server.h"
#include "../adapter/transform.h"
#include "ftp.h"
#include "http.h"

extern TD process_esb_request(char* bmd_file_path);
bool create_worker_thread(int fd);
void log_msg(const char *msg, bool terminate) {
    printf("%s\n", msg);
    if (terminate) exit(-1); /* failure */
}
int make_named_socket(const char *socket_file, bool is_client) {
    printf("Creating AF_LOCAL socket at path %s\n", socket_file);
    if (!is_client && access(socket_file, F_OK) != -1) {
        log_msg("An old socket file exists, removing it.", false);
        if (unlink(socket_file) != 0) {
            log_msg("Failed to remove the existing socket file.", true);
        }
    }
    struct sockaddr_un name;
    /* Create the socket. */
    int sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        log_msg("Failed to create socket.", true);
    }

    /* Bind a name to the socket. */
    name.sun_family = AF_LOCAL;
    strncpy (name.sun_path, socket_file, sizeof(name.sun_path));
    name.sun_path[sizeof(name.sun_path) - 1] = '\0';

    /* The size of the address is
       the offset of the start of the socket_file,
       plus its length (not including the terminating null byte).
       Alternatively you can just do:
       size = SUN_LEN (&name);
   */
    size_t size = (offsetof(struct sockaddr_un, sun_path) +
                   strlen(name.sun_path));
    if (is_client) {
        if (connect(sock_fd, (struct sockaddr *) &name, size) < 0) {
            log_msg("connect failed", 1);
        }
    } else {
        if (bind(sock_fd, (struct sockaddr *) &name, size) < 0) {
            log_msg("bind failed", 1);
        }
    }
    return sock_fd;
}

void thread_function(int sock_fd) {
    log_msg("SERVER: thread_function: starting", false);
    char buffer[5000];
    memset(buffer, '\0', sizeof(buffer));
    
    int count = read(sock_fd, buffer, sizeof(buffer));
    if (count > 0) {
        printf("SERVER: Received from client: %s\n", buffer);
        write(sock_fd, buffer, sizeof(buffer)); /* echo as confirmation */
    }
    
    char pth[50];
    strcpy(pth,"../.");             /* The file path is modified to locate the file properly in correct folder */   
    strcat(pth,buffer);             /* Buffer contains the file path from client */
    printf("path==>%s\n",pth);
    TD st = process_esb_request(pth);

   if(st.val==1)
   {
       printf("BMD file succesfully processed and stored\n");
   }

    if(!(strcmp(st.Transform_key,"Json_file") && !(strcmp(st.Transform_value,"json"))))
    {
        const char * filp = transformjson(pth);

    }

    BMD * bmd1 =  parse_bmd_xml(pth);
   printf("payload in server = %s\n\n",bmd1->bmd_payload->data);
   
   char * payloadString = bmd1->bmd_payload->data;     /* Payload data is stored */


    if(!(strcmp(st.Transport_value,"EMAIL")))
    {
        printf("payload=%s\n",payloadString);
        int emailsent = emailsender(st.Transport_key,payloadString);           /* email id and payload data is passed to email sender function */
        if(emailsent == 1)
        {
            printf("Email sent to %s successfully\n",st.Transport_key);
        }

    }


   if(!(strcmp(st.Transport_value,"FTP")))
   {
        char * URL =st.Transport_key;
        int ftpst = send_ftp_file(URL);         //url is obtained from db table
        if(ftpst==1)
        {
            printf("The file has been succesfully transported via FTP server %s\n",URL);
        }
   }

   if(!(strcmp(st.Transport_value,"HTTP")))
   {
       const char * url_http = st.Transport_key;
       const char * data_msg = bmd1->bmd_payload->data;
       int resp = http_request(url_http, data_msg);
       if(resp==1)
       {
           printf("%s is successfully sent via HTTP %s\n",bmd1->bmd_payload->data, url_http);
       }
       
   }
    
    close(sock_fd); /* break connection */
    log_msg("SERVER: thread_function: Done. Worker thread terminating.", false);
    pthread_exit(NULL); // Must be the last statement
}

/**
 * This function launches a new worker thread.
 * @param sock_fd
 * @return Return true if thread is successfully created, otherwise false.
 */
bool create_worker_thread(int sock_fd) {
    log_msg("SERVER: Creating a worker thread.", false);
    pthread_t thr_id;
    int rc = pthread_create(&thr_id,
            /* Attributes of the new thread, if any. */
                            NULL,
            /* Pointer to the function which will be
             * executed in new thread. */
                            thread_function,
            /* Argument to be passed to the above
             * thread function. */
                            (void *) sock_fd);
    if (rc) {
        log_msg("SERVER: Failed to create thread.", false);
        return false;
    }
    return true;
}

/**
 * Sends a message to the server socket.
 * @param msg Message to send
 * @param socket_file Path of the server socket on localhost.
 */
_Noreturn void start_server_socket(char *socket_file, int max_connects) {
    int sock_fd = make_named_socket(socket_file, false);

    /* listen for clients, up to MaxConnects */
    if (listen(sock_fd, max_connects) < 0) {
        log_msg("Listen call on the socket failed. Terminating.", true); /* terminate */
    }
    log_msg("Listening for client connections...\n", false);
    /* Listens indefinitely */
    while (1) {
        struct sockaddr_in caddr; /* client address */
        int len = sizeof(caddr);  /* address length could change */

        printf("Waiting for incoming connections...\n");
        int client_fd = accept(sock_fd, (struct sockaddr *) &caddr, &len);  /* accept blocks */
        

        if (client_fd < 0) {
            log_msg("accept() failed. Continuing to next.", 0); /* don't terminate, though there's a problem */
            continue;
        }
        /* Start a worker thread to handle the received connection. */
        if (!create_worker_thread(client_fd)) {
            log_msg("Failed to create worker thread. Continuing to next.", 0);
            continue;
        }

    }  
}


int main() {
  
        start_server_socket("./my_sock", 10);           /* start the sever before kodev build and kodev run */
        return 0;
    
}


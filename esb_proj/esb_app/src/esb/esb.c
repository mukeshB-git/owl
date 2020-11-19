#include<stdio.h>
#include "../adapter/email.h"
#include "esb.h"
#include<stdlib.h>
#include<mysql/mysql.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "server.h"



static char *server = "localhost";
static char *user = "root";
static char *password = "Mukesh123@";
static char *database = "esb";

unsigned int port = 3306;
static char *unix_socket = NULL;

unsigned int flag = 0;




static envelope *get_envelope_struct()
{
    envelope *bmd_envelope = (envelope *)malloc(sizeof(envelope));
    return bmd_envelope;
}

static payload *get_payload_struct()
{
    payload *bmd_payload = (payload *)malloc(sizeof(payload));
    return bmd_payload;
}



BMD * parse_bmd_xml(char* bmd_file_path) {
    
    xmlDoc         *document;
    xmlNode        *root, *first_child, *node,*temp;
    char           *filename;
    
    BMD *bmd = (BMD *)(malloc(sizeof(BMD *)));

    bmd->bmd_envelope = get_envelope_struct();
    bmd->bmd_payload = get_payload_struct();




    filename = bmd_file_path;
    
    document = xmlReadFile(filename, NULL, 0);
    temp = xmlDocGetRootElement(document);
    root=xmlFirstElementChild(temp);
  
        char* arrayName[9];
        char* arrayValue[9];
        int i=0;
        first_child = root->children;
        for (node = first_child; node; node = node->next) {

            if(node->type==1)
            {         
                arrayName[i]=(char*)node->name;
                arrayValue[i]=(char*)xmlNodeGetContent(node);
                i++;
            }
        }

    root=xmlLastElementChild(temp);
    first_child = root;
        for (node = first_child; node; node = node->next) {
            if(node->type==1)
            {            
                arrayName[i]=(char*)node->name;            
                arrayValue[i]=(char*)xmlNodeGetContent(node);
                i++;
            }
        }
   
    for(int x=0; x<9; x++){
        if(strcmp(arrayName[x], "MessageID")==0)
            bmd->bmd_envelope->MessageID=(unsigned char*)arrayValue[x];
        if(strcmp(arrayName[x], "MessageType")==0)
            bmd->bmd_envelope->MessageType=(unsigned char*)arrayValue[x];
        if(strcmp(arrayName[x], "Sender")==0)   
            bmd->bmd_envelope->Sender=(unsigned char*)arrayValue[x];
         if(strcmp(arrayName[x], "Destination")==0) 
            bmd->bmd_envelope->Destination=(unsigned char*)arrayValue[x];
        if(strcmp(arrayName[x], "CreationDateTime")==0)
            bmd->bmd_envelope->CreationDateTime=(unsigned char*)arrayValue[x];
        if(strcmp(arrayName[x], "Signature")==0)
            bmd->bmd_envelope->Signature=(unsigned char*)arrayValue[x];
        if(strcmp(arrayName[x], "ReferenceID")==0)    
            bmd->bmd_envelope->ReferenceID=(unsigned char*)arrayValue[x];
    //bmd->bmd_envelope->UserProperties=(unsigned char*)arrayValue[7];
        if(strcmp(arrayName[x], "Payload")==0) 
            bmd->bmd_payload->data=(unsigned char*)arrayValue[x];
    }

 
    return bmd;
}

 TD  is_bmd_valid(BMD * bmd)
{
    TD t1;          //Declare a structure to store transport and transform data
    int valid = 0; // 1 => vaild, -1 => invalid
  
     MYSQL * conn;
    MYSQL_RES * res, * resTf, * resTr;
    MYSQL_ROW row, rowTf, rowTr;
     
      if(*(bmd->bmd_envelope->MessageID)==0)
    {
        printf("MessageId is mandatory (missing in BMD file)\n");
        valid = 0;
    }
     if(*(bmd->bmd_envelope->MessageType)==0)
    {
        printf("MessageType is mandatory (missing in BMD file)\n");
        valid = 0;

    } if(*(bmd->bmd_envelope->Sender)==0)
    {
        printf("Sender Id is mandatory (missing in BMD file)\n");
        valid = 0;

    } if(*(bmd->bmd_envelope->Destination)== 0)
    {      
        printf("Destination Id is mandatory (missing in BMD file)\n");
        valid = 0;

    } if(*(bmd->bmd_envelope->CreationDateTime)== 0)
    {
        printf("CreationDateTime is mandatory (missing in BMD file)\n");
        valid = 0;

    } if(*(bmd->bmd_envelope->Signature)== 0)
    {
        printf("Signature is mandatory (missing in BMD file)\n");
        valid = 0;

    } if(*(bmd->bmd_envelope->ReferenceID)== 0)
    {
        printf("ReferenceID is mandatory (missing in BMD file)\n");
        valid = 0;
    }
    
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        printf("Failed to connect MySQL Server %s. Error: %s\n", server, mysql_error(conn));
        exit(1);
       
    }
    printf("Connection success\n");

    mysql_query(conn,"SELECT sender, destination, message_type, route_id FROM routes;");
    res = mysql_store_result(conn);
    int num_fields = mysql_num_fields(res); //Number of columns in database table

    char query2[5000], query3[5000];
    char * st;
    
    while ((row = mysql_fetch_row(res)))
    {
        for(int i = 0; i < num_fields; i++)
        {
        
        if(!(strcmp(bmd->bmd_envelope->Sender,row[i])) && !(strcmp(bmd->bmd_envelope->Destination,row[i+1])) && !(strcmp(bmd->bmd_envelope->MessageType,row[i+2])))
        {
            printf("ID matched...\n");
            st = row[i+3];  //route id is stored 
            printf("route_id=>%s\n",st);
           valid =1;
           break;
           
        }
      
        
        }
    }
    if(valid==1)
    {
     sprintf(query2,CHECK_TRANSFORM, st);
            printf("\n\n%s\n\n", query2);
            if (mysql_query(conn, query2)) {
                printf("Failed to execute query.Error: %s\n", mysql_error(conn));
            }
            resTf =  mysql_store_result(conn);
            rowTf = mysql_fetch_row(resTf);
                
                if(((rowTf[0]!=NULL)) && ((rowTf[1]!=NULL) ))
                {
                    printf("%s\t%s\n",rowTf[0],rowTf[1]);
                    t1.Transform_key = rowTf[0];
                    t1.Transform_value = rowTf[1];
                        valid = 1;
                       
                }

                else
                {
                    valid=0;
                }
                
    }
    else
    {
        valid=0;
    }
    
    if(valid==1)
    {
     sprintf(query3,CHECK_TRANSPORT, st);
            printf("\n\n%s\n\n", query3);
            if (mysql_query(conn, query3)) {
                printf("Failed to execute query.Error: %s\n", mysql_error(conn));
            }
            resTr =  mysql_store_result(conn);
            rowTr = mysql_fetch_row(resTr);
                
                if(((rowTr[0]!=NULL)) && ((rowTr[1]!=NULL) ))
                {
                    printf("%s\t%s\n",rowTr[0],rowTr[1]);
                    t1.Transport_key = rowTr[0];
                    t1.Transport_value = rowTr[1];
                        valid = 1;
                       
                }

                else
                {
                    valid=0;
                }
    }   

    else
    {
        valid=0;
    }
    

         t1.val=valid;        
    printf("valid=%d\n\n",valid);

    /*free the result*/
    mysql_free_result(resTf);
    mysql_free_result(resTr);
    mysql_free_result(res);
    mysql_close(conn);
    
    return t1;
}

int queue_the_request(BMD * bmd, char * bmd_file_path)
{
    int success = 1; // 1 => OK, -1 => Error cases

    MYSQL * con1;
    MYSQL_RES * res1;
    MYSQL_ROW row;

    int8_t received_time[100]; // eg: "2020-08-12T05:18:00+0000";
    int8_t received_on[100];
    int8_t received_on1[100];
    strcpy(received_time, bmd -> bmd_envelope -> CreationDateTime);

    /*Changing DateTime format*/
    int n = strlen(received_time);
    int j = 0;
    for (int i = 0; i < n-5 ; i++) {
        received_on[j] = received_time[i];
        if (received_on[i] == 'T') //test for character
        {
            received_on[i] = ' '; // change T to space
        }
        if (received_on[i] == '+') //test for character
        {
            break;
        }
        j++;
    }

    con1 = mysql_init(NULL);
    for (int i = 0,j=0; i < 19 ; i++) {
        received_on1[j] = received_on[i];
        if (received_on[i] == 'm') //test for character
        {
            break;
            
        }
        j++;
    }
    printf("\n\ntime==>%s\n\n", received_on);   //checking the time format
    /* Connect to database */
    if (!mysql_real_connect(con1, server, user, password, database, 0, NULL, 0)) {
        printf("Failed to connect MySQL Server %s. Error: %s\n", server, mysql_error(con1));
        success = -1;
        return success;
    }

    /*sql query to insert in table*/
    char * status = "available";
    char query1[5000];
    

    sprintf(query1, INSERT_IN_ESB_REQUEST,
        bmd -> bmd_envelope -> Sender,
        bmd -> bmd_envelope -> Destination,
        bmd -> bmd_envelope -> MessageType,
        bmd -> bmd_envelope -> ReferenceID,
        bmd -> bmd_envelope -> MessageID,
        received_on, bmd_file_path, status);

    printf("\n\n%s\n\n", query1);

    if (mysql_query(con1, query1)) {
        printf("Failed to execute query.Error: %s\n", mysql_error(con1));
        success = -1;
        return success;
    }

    res1 = mysql_use_result(con1);

    /*free the result*/


    mysql_free_result(res1);
    mysql_close(con1);

   

    return success;
}



/**
 * This is the main entry point into the ESB. 
 * It will start processing of a BMD received at the HTTP endpoint.
 */
 TD  process_esb_request(char* bmd_file_path) {
    TD td;  
    int status = 1; // 1 => OK, -ve => Errors
   
    printf("Handling the BMD file %s\n", bmd_file_path);
    
    // Step 1:
    BMD * bmd;
   bmd = parse_bmd_xml(bmd_file_path);

   // Step 2:
    td = (is_bmd_valid(bmd)); 
    if(td.val==0)
    {
        //TODO: Process the error case
        printf("BMD is invalid!\n");
        status = -2;
    }
    else
    {
        // Step 3:
        
        status = queue_the_request(bmd,bmd_file_path);
    }

    td.val=status;

    return td;      // return the structure which contains all the transport and transform data
}
#include<stdio.h>
#include<string.h>


int  emailsender(char *to,  char *text)
{
      printf("entering email func..\n");
      int sent = 0;
     char cmd[1000];  // to hold the command.
     int len = strlen(text);
        char tempFile[len];     // name of tempfile.

        FILE *fp = fopen(tmpfile,"w"); // open it for writing.
        fprintf(fp,"From: Team Owl\r\r\n");
        fprintf(fp,"Subject:Payload\r\n");  
         fprintf(fp,"\r\n");              //one line should be given to separate subject from body of mail
        fprintf(fp,"%s\n",text);        // write body to it.
        fclose(fp);             // close it.
         printf("received=%s\n",text);
        sprintf(cmd,"ssmtp %s < %s",to,tmpfile); // prepare command.
         system(cmd);     // execute it.
         sent =1;
         return sent;
        
}


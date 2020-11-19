#include <stdio.h>
#include <curl/curl.h>
#include "http.h"


static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  curl_off_t nread;
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  size_t retcode = fread(ptr, size, nmemb, stream);

  nread = (curl_off_t)retcode;

  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);
  return retcode;
}


int http_request(const char * h_url, const char * msg)
{
  printf("entering http func url=%s msg=%s\n",h_url,msg);
  int status=0;
  
  CURL *curl;
  CURLcode res;
 
  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, h_url);
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

    /* Perform the request, res will get the return code */

    res = curl_easy_perform(curl);

    long response;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);
    /*
     If Response from Http url is 2XX then Successful transport
    */
    if(response >199 && response <300 )
    {
      printf("\nSuccesfully sent the request via HTTP, response code = %ld\n",response);
      status =1;
    }
    printf("response=%ld\n",response);


    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return status;
}
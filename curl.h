#ifndef _CURL_H_
#define _CURL_H_

#define MXCHIP_AGENT "mxchip-agent/1.0"
#define BUF_LEN 1024

typedef size_t (*curl_read_callback)(char *buffer,
                                      size_t size,
                                      size_t nitems,
                                      void *instream);

typedef struct {

    int fd;
    int port;
    char *postdata;
    char *url;  // exclude host name.
    char *host;
    char *agent;
    void *instream;
    curl_read_callback cb;
    char buf[BUF_LEN];
} CURL;

typedef enum {
    CURLOPT_URL,
    CURLOPT_USERAGENT,
    CURLOPT_WRITEFUNCTION,
    CURLOPT_WRITEDATA,
    CURLOPT_METHOD_POST,
    
    CURLOPT_LASTENTRY /* the last unused */
} curl_opt_e;

CURL *curl_easy_init();
int curl_easy_setopt(CURL *curl, curl_opt_e type, void *arg);
int curl_easy_perform(CURL *curl);
void curl_easy_cleanup(CURL*curl);
#endif

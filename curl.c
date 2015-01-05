#include "stdlib.h"
#include "string.h"
//#include "mxchipWNET.h"
#include "MICO.h"
#include "Platform.h"
#include "MICOSocket.h"
#include "SocketUtils.h"
#include "curl.h"

static int debug_url = 0;

/* Find dst string from src string. return the first place */
char *memmem(char *src, int src_len, const char *dst, int dst_len)
{
  int i, j;
  
  for (i=0; i<=src_len-dst_len; i++) {
    if (src[i] == dst[0]) {
      for (j=1; j<dst_len; j++) {
        if (src[i+j] != dst[j])
          break;
      }
      if (j == dst_len)
        return &src[i];
    }
  }
  
  return NULL;
}

CURL *curl_easy_init()
{
    CURL *ret;

    ret = (CURL*)malloc(sizeof(CURL));

    memset(ret, 0, sizeof(CURL));
    ret->agent = MXCHIP_AGENT;
    ret->fd = -1;
    return ret;
}

static int decode_http_url(CURL *curl, char*url)
{
    int i=0;
    char *p, *host;

    if (memcmp(url, "http://", strlen("http://")) == 0)
        host = url + strlen("http://");
    else
        return 0;
    
    p = host;
    while((p[i] != '/') && (p[i]!= '\0'))
        i++;
    p[i] = '\0';

    curl->host = host;
    curl->url = &p[i+1];

    return 1;
}

int curl_easy_setopt(CURL *curl, curl_opt_e type, void *arg)
{
    switch(type) {
    case CURLOPT_URL:
        decode_http_url(curl, (char*)arg);
        curl->port = 80;
        break;
    case CURLOPT_USERAGENT:
        curl->agent = arg;
        break;
    case CURLOPT_WRITEFUNCTION:
        curl->cb = (curl_read_callback)arg;
        break;
    case CURLOPT_WRITEDATA:
        curl->instream = arg;
        break;
    case CURLOPT_METHOD_POST:
        curl->postdata = (char*)arg;
        break;
    default:
        break;
    }
    return 0;
}

static const char endline[] = {'\r', '\n', '\r', '\n'};
static const char lengthstr[] = "Content-Length: ";

/* return status code */
static int HTTPHeaderParse( char *src, int *contentLength)
{
    int   x, i;
    int   len = strlen(src);
    char  *lengthpos;
    int statusCode;

    for(i=0; i<len; i++) {
        if (src[i] == ' ')
            break;
    }

    i++;

    statusCode = atoi(&src[i]);

    if ( ( lengthpos = (char*) memmem( src,  len, lengthstr, strlen( lengthstr )) ) != NULL )
    {
        *contentLength = atoi(lengthpos + sizeof(lengthstr)-1);
    }

    return statusCode;
}

int curl_easy_perform(CURL *curl)
{
    int fd;
    struct sockaddr_t addr;
    char ip_str[16], *endpos;
    int len, content_len, status_code;
    int offset = 0;
    int i = 0;

    while(kNoErr != gethostbyname(curl->host, ip_str, 16)) {
        i++;
        if (i > 10)
            return 0;
    }
    if (debug_url)
        printf("%s ip %s\r\n", curl->host, ip_str);
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    addr.s_ip = inet_addr(ip_str);
    addr.s_port = curl->port;
    if (connect(fd, &addr, sizeof(addr))!=0) {
        if (debug_url)
            printf("could not connect\r\n");
        close(fd);
        return 0;
    }
    else{
        if (curl->postdata == NULL)
            sprintf(curl->buf, "GET /%s HTTP/1.1\r\nUser-Agent: %s\r\nHOST: %s\r\nAccept: */*\r\n\r\n",
                curl->url, curl->agent, curl->host);
        else
            sprintf(curl->buf, "POST /%s HTTP/1.0\r\nUser-Agent: %s\r\nHOST: %s\r\nAccept: */*Connection: Keep-Alive\r\n"
                "Content-Type: application/x-www-form-urlencoded\r\n"
                "Content-Length: %d\r\n\r\n%s\r\n",
                curl->url, curl->agent, curl->host, strlen(curl->postdata), curl->postdata);

        //printf("===send===\r\n%s", curl->buf);
        send(fd, curl->buf, strlen(curl->buf), 0);
        len = recv(fd, curl->buf, BUF_LEN, 0);
        //printf("===recv===\r\n%s", curl->buf);
        status_code = HTTPHeaderParse(curl->buf, &content_len);
        if (debug_url)
            printf("status code %d, content_len %d\r\n", status_code, content_len);
        if (status_code != 200) {
            
            close(fd);
            return 0;
        }
        endpos = (char*) memmem(curl->buf, len, endline, sizeof(endline));
        if (endpos < (curl->buf + len)) {
            if (debug_url)
                printf("len %d, buf %p, endpos %p\r\n", len,curl->buf,endpos);
            len = curl->buf + len - endpos - 4;
            content_len -= len;
            if (debug_url)
                printf("len %d\r\n", len);
            offset = curl->cb(endpos+4, 1, len, curl->instream);
            for(i=0; i<len-offset; i++) {
                curl->buf[i] = curl->buf[offset+i];
            }
            offset = i;
        }
    }

    while(1) {
        len = recv(fd, &curl->buf[offset], BUF_LEN-offset, 0);
        content_len -= len;
        if (debug_url)
            printf("offset %d, recv %d\r\n", offset, len);
        if (len > 0) {
            offset = curl->cb(curl->buf, 1, len+offset, curl->instream);
            for(i=0; i<len-offset; i++) {
                curl->buf[i] = curl->buf[offset+i];
            }
            offset = i;
            if (content_len <= 0)
                break;
        } else
            break;
    }
    close(fd);
    return 1;
}

void curl_easy_cleanup(CURL*curl)
{
    if (curl)
        free(curl);
}


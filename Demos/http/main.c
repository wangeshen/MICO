#include "stdio.h"
#include "stdarg.h"
#include "ctype.h"

#include "MICO.h"
#include "micocli.h"
#include "URLUtils.h"

typedef enum {
  NOTIFY_STATION_UP = 1,
  NOTIFY_STATION_DOWN,

  NOTIFY_AP_UP,
  NOTIFY_AP_DOWN,
} WiFiEvent;

/*
  * while(1) {easylink;scan;startnetwork;}
  *
  */

static int wifi_up = 0, easylink_done = 0, scan_done = 0;
static network_InitTypeDef_st wconf;


#define SSL_RECV_BUF_SIZE            (3*1024)
#define MAX_SIZE_URL                 512
#define MAX_SIZE_DOMAIN_NAME         128
#define MAX_SIZE_HTTP_REQUEST_BUF    512

#define HTTP_TCP_PORT                80
#define HTTPS_TCP_PORT               443

typedef void * ssl_t;

char request_url[MAX_SIZE_URL] = {0};
bool need_http_request = false;

/**
  * CALLBACK functions
  *
*/
void ApListCallback(ScanResult *pApList)
{
    int i;
    
    printf("got %d AP\r\n", pApList->ApNum);
    for(i=0; i<pApList->ApNum; i++) {
        printf("\t%s %d\r\n", pApList->ApList[i].ssid, pApList->ApList[i].ApPower);
    }
    scan_done = 1;
}

void ApListAdvCallback(ScanResult_adv *pApAdvList)
{
  scan_done = 1;
}

void WifiStatusHandler(WiFiEvent status)
{
  if (NOTIFY_STATION_UP == status){
        wifi_up = 1;
        printf("wifi on, now can do http/https\r\n\tusage: http <full_url>\r\n");
  }
  else if (NOTIFY_STATION_DOWN == status){
        wifi_up = 0;
  }
}

void connected_ap_info(apinfo_adv_t *ap_info, char *key, int key_len)
{
    printf("%s, ap BSSID %02x-%02x-%02x-%02x-%02x-%02x, key %s, key_len %d\r\n",
        __FUNCTION__, ap_info->bssid[0], ap_info->bssid[1], 
        ap_info->bssid[2], ap_info->bssid[3], 
        ap_info->bssid[4], ap_info->bssid[5], 
        key, key_len);
}

void NetCallback(IPStatusTypedef *pnet)
{
    printf("%s, ip %s, gw %s, dns %s, netmask %s\r\n", __FUNCTION__, pnet->ip, 
        pnet->gate, pnet->dns, pnet->mask);
}

void RptConfigmodeRslt(network_InitTypeDef_st *nwkpara)
{
    if (nwkpara) {
        memcpy(&wconf, nwkpara, sizeof(wconf));
        printf("Easylink success: %s-%s\r\n", wconf.wifi_ssid, wconf.wifi_key);
    } else {
        printf("Easylink Fail\r\n");
    }
    easylink_done = 1;
}

void easylink_user_data_result(int datalen, char*data)
{
  
}

void socket_connected(int fd)
{
  
}

void dns_ip_set(uint8_t *hostname, uint32_t ip)
{
  
}


void system_version(char *str, int len)
{
  
}


void join_fail(OSStatus err)
{
    printf("join fail %d\r\n", err);
}

void wifi_reboot_event(void)
{
  
}

void mico_rtos_stack_overflow(char *taskname)
{
 
}

static void ez_Command(char *pcWriteBuffer, int xWriteBufferLen,int argc, char **argv)
{
    if (argc == 1) {
        cmd_printf("Start Easylink...\r\n");
        micoWlanStartEasyLink(20);
    } else {
        cmd_printf("Stop Easylink...\r\n");
        micoWlanStopEasyLink();
    }
}

static void connect_Command(char *pcWriteBuffer, int xWriteBufferLen,int argc, char **argv)
{
    if (argc == 1) {
        cmd_printf("Usage: conn <ssid> [key]\r\n");
        return;
    }

    memset(&wconf, 0x0, sizeof(network_InitTypeDef_st));
    strcpy(wconf.wifi_ssid, argv[1]);
    if (argc == 3)
        strcpy(wconf.wifi_key, argv[2]);

    wconf.wifi_mode = Station;
    wconf.dhcpMode = DHCP_Client;
    wconf.wifi_retry_interval = 100;


    micoWlanStart(&wconf);
  
    printf("connect to %s (%s)\r\n", wconf.wifi_ssid, wconf.wifi_key);

}

static void http_Command(char *pcWriteBuffer, int xWriteBufferLen,int argc, char **argv)
{
    if (argc == 1) {
        cmd_printf("Usage: http <full_url>\r\n");
        return;
    }
    
    memset(request_url, 0, MAX_SIZE_URL);
    strncpy(request_url, argv[1], MAX_SIZE_URL);
    need_http_request = true;
    cmd_printf("http client test:\t request %s ...", request_url);
}

// easylink, airkiss, connect, softap, tcp ip port, tcp tx, tcp echo,
static const struct cli_command user_clis[] = {
    {"easylink", "Start Easylink",              ez_Command},
    {"conn",     "Connect AP",                  connect_Command},
    {"http",     "http test",                   http_Command}
};

void http_client_thread(void *arg)
{
  int fd_client = -1;
  char *buf, ipstr[32];
  struct sockaddr_t addr;
  ssl_t *ssl;
  int errno = 0;
  OSStatus err = kUnknownErr;
  
  URLComponents urlComponents;
  char request_host[MAX_SIZE_DOMAIN_NAME] = {0};
  char *request_path = NULL;
  char http_Request[MAX_SIZE_HTTP_REQUEST_BUF] = {0};
  bool is_https = false;
  int ret = 0;
  
  buf = (char*)malloc(SSL_RECV_BUF_SIZE);
  
  while(1) {
    sleep(1);

    if(wifi_up) {  // If wifi is established 
      if(need_http_request){  // http request
        need_http_request = false;
        // get secheme && host && path from url
        err = URLParseComponents(request_url, NULL, &urlComponents, NULL);
        if(kNoErr != err){
          printf("http client test:\t ERROR: URLParseComponents failed! err=%d\r\n", err);
          continue;
        }
        
        memset((void*)request_host, 0, MAX_SIZE_DOMAIN_NAME);
        if( (NULL == urlComponents.hostPtr) || (urlComponents.hostLen > MAX_SIZE_DOMAIN_NAME) ){
          printf("http client test:\t ERROR: URLParseComponents host err!\r\n");
          continue;
        }
        else{
          strncpy(request_host, urlComponents.hostPtr , urlComponents.hostLen);
          request_path = (char*)urlComponents.pathPtr;
          if(0 == strlen(request_path)){
            request_path = "/";
          }
          printf("http client test:\t host=%s, path=%s\r\n", 
                 request_host, request_path);
        }
        
        // url scheme: http/https
        if(0 == strncmp("https", urlComponents.schemePtr, strlen("https"))){
          is_https = true;
        }
        else {
          is_https = false;
        }
        
        // get host ip addr
        printf("http client test:\t connecting to %s\r\n", request_host);
        if(gethostbyname(request_host, (uint8_t *)ipstr, 32) != 0){
          printf("http client test:\t get %s failed! Redo DNS in 5 sec... \r\n", request_host);	
          need_http_request = true;
          sleep(5);
          continue;
        }
        
        // socket connect
        printf("http client test:\t %s address is %s \r\n", request_host, ipstr);			
        fd_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        addr.s_ip = inet_addr(ipstr); 
        if(is_https){
          printf("http client test:\t port = %d\r\n", HTTPS_TCP_PORT);
          addr.s_port = HTTPS_TCP_PORT;  
        }
        else {
          printf("http client test:\t port = %d\r\n", HTTP_TCP_PORT);
          addr.s_port = HTTP_TCP_PORT;  
        }
        
        if (connect(fd_client, &addr, sizeof(addr))!=0) {
          close(fd_client);
          fd_client = -1;
          need_http_request = true;
          printf("http client test:\t socket connect failed! Reconnect in 5 sec...\r\n");
          sleep(5);
          continue;
        }
        else{
          printf("http client test:\t socket connect success, fd: %d\r\n", fd_client);
          // create http request data
          memset(http_Request, 0, MAX_SIZE_HTTP_REQUEST_BUF);
          sprintf(http_Request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",
                  request_path, request_host);
          
          // send request
          if(is_https){
            printf("http client test:\t ssl connect...\r\n");
            ssl = (void*)ssl_connect(fd_client, 0, NULL, &errno);
            if (ssl == NULL){
              printf("http client test:\t ssl fail, err=%d\r\n", errno);
            }
            else {
              // send request by ssl
              printf("http client test:\t ssl connected.\r\n");
              printf("http client test:\t send data: \r\n%s\r\n", http_Request);
              ret = ssl_send(ssl, (char*)http_Request, strlen(http_Request));
              if(strlen(http_Request) != ret){
                printf("http client test:\t ssl send err = %d.\r\n", ret);
                ssl_close(ssl);
                close(fd_client);
                fd_client = -1;
                need_http_request = true;
                continue;
              }
              // recv data
              ssl_recv(ssl, buf, SSL_RECV_BUF_SIZE);
              buf[SSL_RECV_BUF_SIZE] = 0;
              printf("http client test:\t recv data:\r\n%s\r\n", buf);
              ssl_close(ssl);
              printf("http client test:\t ssl closed.\r\n");
            }
          }
          else {
            // send request by socket
            printf("http client test:\t send data:\r\n%s\r\n", http_Request);
            ret = send(fd_client, (char*)http_Request, strlen(http_Request), 0);
            if(strlen(http_Request) != ret){
              printf("http client test:\t socket send err = %d.\r\n", ret);
              close(fd_client);
              fd_client = -1;
              need_http_request = true;
              continue;
            }
            // recv data
            recv(fd_client, buf, SSL_RECV_BUF_SIZE, 0);
            buf[SSL_RECV_BUF_SIZE] = 0;
            printf("http client test:\t recv data:\r\n%s\r\n", buf);
          }
          
          // close socket
          close(fd_client);
          fd_client = -1;
          printf("http client test:\t socket closed.\r\n");
          need_http_request = false;
          continue;
        }
      }
      else{
      }
    }	
  }
}

void application_start(void)
{
    printf("hello MICO\r\n");
    MicoInit();
    MicoCliInit();
    cli_register_commands(user_clis, sizeof(user_clis)/sizeof(struct cli_command));

    printf("set AP\r\n\tusage: \"conn <SSID> <KEY>\"\r\n");
    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "http_client", http_client_thread,
            0x1000, 0);
    mico_rtos_delete_thread(NULL);
}



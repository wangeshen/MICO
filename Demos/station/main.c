#include "stdio.h"
#include "stdarg.h"
#include "ctype.h"

#include "MICO.h"

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
    if (NOTIFY_STATION_UP == status) {
        printf("wifi up\r\n");
        wifi_up = 1;
    } else if (NOTIFY_STATION_DOWN == status) {
        printf("wifi down\r\n");
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


void stationModeStart(void)
{
    int mode;
    network_InitTypeDef_st wNetConfig;
    memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    wNetConfig.wifi_mode = Station;

    strcpy((char*)wNetConfig.wifi_ssid, "TP-LINK_5C26");
    strcpy((char*)wNetConfig.wifi_key, "88888888");
    wNetConfig.dhcpMode = DHCP_Client;
    wNetConfig.wifi_retry_interval = 100;
    strcpy((char*)wNetConfig.local_ip_addr, "192.168.2.15");
    strcpy((char*)wNetConfig.net_mask, "255.255.255.0");
    strcpy((char*)wNetConfig.gateway_ip_addr, "192.168.2.1");
    strcpy((char*)wNetConfig.dnsServer_ip_addr, "8.8.8.8");

    micoWlanStart(&wNetConfig);
  
    printf("connect to %s.....\r\n", wNetConfig.wifi_ssid);
}

#define BUF_LEN (3*1024)

static void tcp_server_thread(void *arg)
{
  int i, j,con, len, fd_listen = -1;
  struct sockaddr_t addr;
  fd_set readfds;
  struct timeval_t t;
  char *buf, ip_address[16];
  int clientfd[8];
  int opt;
  
  for(i=0;i<8;i++) 
    clientfd[i] = -1;
  
  buf = (char*)malloc(BUF_LEN);
  
  t.tv_sec = 1;
  t.tv_usec = 0;
  
  /*Establish a TCP server that accept the tcp clients connections*/
  if (fd_listen==-1) {
    fd_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.s_ip = INADDR_ANY;
    addr.s_port = 8080;
    bind(fd_listen, &addr, sizeof(addr));
    listen(fd_listen, 0);
    printf("TCP server test: TCP server established at port: %d\r\n", addr.s_port);
  }
    opt = 2;
    setsockopt(fd_listen, IPPROTO_TCP, TCP_MAX_CONN_NUM, &opt, sizeof(opt));

    getsockopt(fd_listen, IPPROTO_TCP, TCP_MAX_CONN_NUM, &opt, &len);
    printf("opt %d, len %d\r\n", opt, len);
    len = 4;
    opt = -1;
    i = getsockopt(fd_listen, SOL_SOCKET, SO_ERROR, (char *) &opt, &len);
    printf("get error opt ret %d: %d, len %d\r\n", i, opt, len);
  while(1){
    /*Check status on erery sockets */
    FD_ZERO(&readfds);
    FD_SET(fd_listen, &readfds);	
    for(i=0;i<8;i++) {
      if (clientfd[i] != -1)
        FD_SET(clientfd[i], &readfds);
    }
    
    select(1, &readfds, NULL, NULL, &t);
    
    /*Check tcp connection requests */
    if(FD_ISSET(fd_listen, &readfds)){
      j = accept(fd_listen, &addr, &len);
      if (j > 0) {
        inet_ntoa(ip_address, addr.s_ip );
        printf("TCP server test: Client %s:%d connected, fd: %d\r\n", ip_address, addr.s_port, j);
        for(i=0;i<8;i++) {
          if (clientfd[i] == -1) {
            clientfd[i] = j;
            break;
          }
        }
      }
    }
    
    /*Read data from tcp clients and send data back */ 
    for(i=0;i<8;i++) {
      if (clientfd[i] != -1) {
        if (FD_ISSET(clientfd[i], &readfds)) {
          con = recv(clientfd[i], buf, BUF_LEN, 0);
          if (con > 0) {
            send(clientfd[i], buf, con, 0);
          }
          else {
            close(clientfd[i]);
            printf("TCP server test: Client closed, fd: %d\r\n", clientfd[i]);
            clientfd[i] = -1;
          }
        }
      }
    }

  }
}


void application_start(void)
{
    printf("Station with TCP server 8080\r\n");
    
    MicoInit();
    stationModeStart();
    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "TCPSERVER", tcp_server_thread,
            2048, 0);
    while(1) {
        msleep(100);
        
    }
}



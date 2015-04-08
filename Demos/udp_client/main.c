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
    if (NOTIFY_STATION_UP == status)
        wifi_up = 1;
    else if (NOTIFY_STATION_DOWN == status)
        wifi_up = 0;
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


#define BUF_LEN (2*1024)
void udp_echo_thread(void *arg)
{
  int fd_udp = -1, con;
  char *buf;
  struct timeval_t t;
  fd_set readfds;
  struct sockaddr_t addr;
  socklen_t addrLen;
  
  buf = (char*)malloc(BUF_LEN);
  t.tv_sec = 10;
  t.tv_usec = 0;
  
  
  /*Establish a UDP port to receive any data sent to this port*/
  if (fd_udp==-1) {
    fd_udp = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
    addr.s_ip = INADDR_ANY;
    addr.s_port = 8090;
    bind(fd_udp, &addr, sizeof(addr));
    printf("UDP Echo test: Open UDP port %d\r\n", addr.s_port);
  }
  
  while(1){
    /*Check status on erery sockets */
    FD_ZERO(&readfds);	
    FD_SET(fd_udp, &readfds);	
    
    select(1, &readfds, NULL, NULL, &t);
    
    /*Read data from udp and send data back */ 
    if (FD_ISSET(fd_udp, &readfds)) {
      con = recvfrom(fd_udp, buf, BUF_LEN, 0, &addr, &addrLen);
      sendto(fd_udp, buf, con, 0, &addr, sizeof(struct sockaddr_t));
    }
  }
}


void application_start(void)
{
    printf("hello MICO\r\n");
    MicoInit();
    while(1) {
        
        printf("=======================\r\nstart easylink...\r\n");
        easylink_done = 0;
        micoWlanStartEasyLink(60);
        while(easylink_done == 0)
            msleep(100);
        printf("easylink done, scan network\r\n");

        scan_done = 0;
        micoWlanStartScan();
        while(scan_done == 0)
            msleep(100);
        
        wifi_up = 0;
        printf("start network\r\n");
        micoWlanStart(&wconf);
        while(wifi_up == 0)
            msleep(100);
        printf("wifi up\r\n");
    }
}



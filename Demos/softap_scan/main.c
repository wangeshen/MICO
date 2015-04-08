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


static void softAPModeStart(void)
{
    network_InitTypeDef_st wNetConfig;
    memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    wNetConfig.wifi_mode = Soft_AP;
    strcpy((char*)wNetConfig.wifi_ssid, "8801_softap");
    strcpy((char*)wNetConfig.wifi_key, "12345678");
    strcpy((char*)wNetConfig.local_ip_addr, "192.168.0.1");
    strcpy((char*)wNetConfig.net_mask, "255.255.255.0");
    strcpy((char*)wNetConfig.gateway_ip_addr, "192.168.0.1");
    wNetConfig.dhcpMode = DHCP_Server;
    micoWlanStart(&wNetConfig);
    printf("Setup soft AP: %s\r\n", wNetConfig.wifi_ssid);
}

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
        printf("Easylink success: %s-%s\r\n", nwkpara->wifi_ssid, nwkpara->wifi_key);
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





void application_start(void)
{
    printf("SoftAP & scan Test\r\n");
    MicoInit();

    softAPModeStart();
    while(1) {
        
        printf("=======================\r\nstart scan...\r\n");

        scan_done = 0;
        micoWlanStartScan();
        while(scan_done == 0)
            msleep(100);

    }
}



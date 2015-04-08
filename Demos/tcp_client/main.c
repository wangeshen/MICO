#include "stdio.h"
#include "stdarg.h"
#include "ctype.h"

#include "MICO.h"
#include "micocli.h"

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

static int  tcp_mode=0, udp_mode=0;// 0=rx, 1=tx, 2=echo.
static char remote_addr[64];
static int  remote_port, udp_port;
static int  tcp_client_running = 0, udp_running=1;
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

static void as_Command(char *pcWriteBuffer, int xWriteBufferLen,int argc, char **argv)
{
    if (argc == 1) {
        cmd_printf("Start Airkiss...\r\n");
        micoWlanStartAirkiss(40);
    } else {
        cmd_printf("Stop Airkiss...\r\n");
        micoWlanStopAirkiss();
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

static void softap_Command(char *pcWriteBuffer, int xWriteBufferLen,int argc, char **argv)
{
    if (argc == 1) {
        cmd_printf("Usage: ap <ssid> [key]\r\n");
        return;
    }

    memset(&wconf, 0x0, sizeof(network_InitTypeDef_st));
    strcpy(wconf.wifi_ssid, argv[1]);
    if (argc == 3)
        strcpy(wconf.wifi_key, argv[2]);

    wconf.wifi_mode = Soft_AP;
    wconf.dhcpMode = DHCP_Server;
    wconf.wifi_retry_interval = 100;
    strcpy((char*)wconf.local_ip_addr, "192.168.0.1");
    strcpy((char*)wconf.net_mask, "255.255.255.0");
    strcpy((char*)wconf.gateway_ip_addr, "192.168.0.1");
    micoWlanStart(&wconf);
  
    printf("SoftAP %s (%s)\r\n", wconf.wifi_ssid, wconf.wifi_key);

}

static void tcp_Command(char *pcWriteBuffer, int xWriteBufferLen,int argc, char **argv)
{
    int i;
    
    if (argc == 3) {
        strcpy(remote_addr, argv[1]);
        remote_port = atoi(argv[2]);
        cmd_printf("connect %s:%d\r\n", remote_addr, remote_port);
        tcp_mode = 0;
        if (tcp_client_running == 1) {
            tcp_client_running = 0;
            i = 0;
            while(tcp_client_running == 0) {
                msleep(100);
                i++;
                if (i>= 10)
                    break;
            }
        }
        tcp_client_running = 1;
        return;
    } else {
        if (strcmp(argv[1], "tx") == 0) {
            tcp_mode = 1;
            return;
        } else if (strcmp(argv[1], "echo") == 0) {
            tcp_mode = 2;
            return;
        } else if (strcmp(argv[1], "rx") == 0) {
            tcp_mode = 0;
            return;
        } else if (strcmp(argv[1], "close") == 0) {
            tcp_client_running = 0;
            return;
        }
    }
}

static void udp_Command(char *pcWriteBuffer, int xWriteBufferLen,int argc, char **argv)
{
    if (strcmp(argv[1], "tx") == 0) {
        udp_mode = 1;
        return;
    } else if (strcmp(argv[1], "echo") == 0) {
        udp_mode = 2;
        return;
    } else if (strcmp(argv[1], "rx") == 0) {
        udp_mode = 0;
        return;
    }
}

static void uptime_Command(char *pcWriteBuffer, int xWriteBufferLen,int argc, char **argv)
{
    cmd_printf("UP time %dms\r\n", mico_get_time());
}

// easylink, airkiss, connect, softap, tcp ip port, tcp tx, tcp echo,
static const struct cli_command user_clis[] = {
    {"easylink", "Start Easylink",              ez_Command},
    {"airkiss",  "Start Airkiss",               as_Command},
    {"conn",     "Connect AP",                  connect_Command},
    {"ap",       "Start SoftAP",                softap_Command},
    {"tcp",      "tcp ip port, tx, echo, rx",   tcp_Command},
    {"udp",      "udp tx, echo, rx",            udp_Command},
    {"time",     "system time",                 uptime_Command},
};

#define BUF_LEN (3*1024)

static void tcp_client_thread(void *arg)
{
  int fd_client = -1;
  char *buf, ipstr[32];
  int con = -1;
  fd_set readfds;
  struct timeval_t t;
  struct sockaddr_t addr;
  int len, opt, ret;
  int tcp_need_connect = 1;
  
  buf = (char*)malloc(BUF_LEN);
  
  t.tv_sec = 0;
  t.tv_usec = 1000;
  
  while(1) { 
    if (tcp_client_running == 0) {
        if (fd_client >= 0) {
            close(fd_client);
            fd_client = -1;
            tcp_need_connect = 1;
        }
        tcp_client_running = -1;
    }
    if (tcp_client_running != 1) {
        msleep(100);
        continue;
    }
    
    /*If wifi is established, connect to www.baidu.com, and send a http request*/
    if(wifi_up&&tcp_need_connect){
      tcp_need_connect = 0;
      printf("TCP client test: Connecting to %s\r\n", remote_addr);
      if(gethostbyname(remote_addr, (uint8_t *)ipstr, 32) != 0){
        printf("TCP client test: %s failed! Redo DNS in 5 sec... \r\n", remote_addr);	
        tcp_need_connect = 1;
        sleep(5);
        continue;
      }

      printf("TCP client test: %s address is %s \r\n", remote_addr, ipstr);			
      fd_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      addr.s_ip = inet_addr(ipstr); 
      addr.s_port = remote_port;
      if (connect(fd_client, &addr, sizeof(addr))!=0) {
        close(fd_client);
        fd_client = -1;
        tcp_need_connect = 1;
        printf("TCP client test: Connect to %s failed! Reconnect in 5 sec...\r\n", remote_addr);
        sleep(5);
        continue;
      }
      else {
        printf("TCP Connected\r\n");
      }
    }	
    
    /*Reset tcp client connection if wifi is down.*/
    if(wifi_up == 0 && fd_client!=-1){
      printf("TCP client test: Wi-Fi down, clean TCP sockets resource.\r\n");
      close(fd_client);
      fd_client = -1;
      tcp_need_connect = 1;
      sleep(1);
      continue;
    }

    if (fd_client == -1)
        continue;
    /*Check status on erery sockets */
    FD_ZERO(&readfds);	
    FD_SET(fd_client, &readfds);
    
    select(1, &readfds, NULL, NULL, &t);

    if(FD_ISSET(fd_client, &readfds)) {
        con = recv(fd_client, buf, BUF_LEN, 0);
        if(con > 0) {
            if (tcp_mode == 2)
                send(fd_client, buf, con, 0);
        } else{
            len = 4;
            ret = getsockopt(fd_client, SOL_SOCKET, SO_ERROR, (char *) &opt, &len);
            printf("get error opt ret %d: %d, len %d\r\n", ret, opt, len);
            close(fd_client);
            fd_client = -1;
            tcp_need_connect = 1;
            printf("TCP client closed.");
        }
    }
    if (tcp_mode == 1) {
        send(fd_client, buf, BUF_LEN, 0);
    }
  }
}

void udp_thread(void *arg)
{
  int fd_udp = -1, con;
  char *buf;
  struct timeval_t t;
  fd_set readfds;
  struct sockaddr_t addr;
  socklen_t addrLen;
  
  buf = (char*)malloc(BUF_LEN);
  t.tv_sec = 0;
  t.tv_usec = 1000;
  
  
  /*Establish a UDP port to receive any data sent to this port*/
  if (fd_udp==-1) {
    fd_udp = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
    addr.s_ip = INADDR_ANY;
    addr.s_port = 10000;
    bind(fd_udp, &addr, sizeof(addr));
    printf("UDP Echo test: Open UDP port %d\r\n", addr.s_port);
  }
  
  while(1){
    if (udp_running == 0) {
        msleep(100);
        continue;
    }
    /*Check status on erery sockets */
    FD_ZERO(&readfds);	
    FD_SET(fd_udp, &readfds);	
    
    select(1, &readfds, NULL, NULL, &t);
    
    /*Read data from udp and send data back */ 
    if (FD_ISSET(fd_udp, &readfds)) {
      con = recvfrom(fd_udp, buf, BUF_LEN, 0, &addr, &addrLen);
      if (udp_mode == 2)
        sendto(fd_udp, buf, con, 0, &addr, sizeof(struct sockaddr_t));
    }
    if (udp_mode == 1)
        sendto(fd_udp, buf, BUF_LEN, 0, &addr, sizeof(struct sockaddr_t));
  }
}

void application_start(void)
{
    printf("hello MICO\r\n");
    MicoInit();
    MicoCliInit();
    cli_register_commands(user_clis, sizeof(user_clis)/sizeof(struct cli_command));

    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "tcpclient", tcp_client_thread,
            2048, 0);
    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "udp", udp_thread,
            2048, 0);
    mico_rtos_delete_thread(NULL);
}



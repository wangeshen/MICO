/**
  ******************************************************************************
  * @file    MVDCloudTest.c 
  * @author  WangEshen
  * @version V1.0.0
  * @date    22-Jan-2015
  * @brief   MVD cloud test.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
  ******************************************************************************
  */ 

#include <stdio.h>

#include "MVDCloudTest.h"
#include "MicoVirtualDevice.h"
#include "EasyCloudUtils.h"
#include "SocketUtils.h"
#include "StringUtils.h"

#define mvd_cloud_test_log(M, ...) custom_log("MVD_CLOUD_TEST", M, ##__VA_ARGS__)
#define mvd_cloud_test_log_trace() custom_log_trace("MVD_CLOUD_TEST")


OSStatus MVDCloudTest_StartRecv(const char* product_id,
                                uint32_t msg_length, 
                                uint32_t period_s, uint32_t interval_ms)
{
  OSStatus err = kUnknownErr;
  int tcpClient_fd = -1;
  char ipstr[16];
  struct sockaddr_t addr;
  fd_set readfds;
  struct timeval_t t;
  
  /* create http request data */
  uint8_t *httpRequestData = NULL;
  size_t httpRequestDataLen = 0;
  HTTPHeader_t *httpHeader = NULL;
  
  const char* host = EASYCLOUD_TEST_SERVER;
  uint16_t port = EASYCLOUD_TEST_PORT;
  char* request_url = NULL;
  uint32_t request_url_len = 0;
  
  // set request_url = "/startTest?id=xxx&length=xxx&deadline=xxx&interval=xxx"
  char *param_productId = "?id=";
  char *param_length = "&length=";
  char *param_period = "&deadline=";
  char *param_interval = "&interval=";
  
  char string_msgLen[32] = {0};
  char string_periodLen[32] = {0};
  char string_intervalLen[32] = {0};
  
  uint32_t productIdLen = 0;
  uint32_t msgLengthLen = 0;
  uint32_t periodLen = 0;
  uint32_t intervalLen = 0;
  
  productIdLen = strlen(product_id);
  Int2Str((uint8_t*)string_msgLen, msg_length);
  msgLengthLen = strlen(string_msgLen);
  Int2Str((uint8_t*)string_periodLen, period_s);
  periodLen = strlen(string_periodLen);
  Int2Str((uint8_t*)string_intervalLen, interval_ms);
  intervalLen = strlen(string_intervalLen);
  
  request_url_len = strlen(EASYCLOUD_TEST_URL_START_RECV) + \
                          strlen(param_productId) + productIdLen + \
                          strlen(param_length) + msgLengthLen + \
                          strlen(param_period) + periodLen + \
                          strlen(param_interval) + intervalLen;
 
  request_url = (char*)malloc(request_url_len+1);
  if (NULL == request_url){
    goto exit;
  }
  memset(request_url, 0, request_url_len+1);
  strncpy(request_url, EASYCLOUD_TEST_URL_START_RECV, strlen(EASYCLOUD_TEST_URL_START_RECV));
  strncat(request_url, param_productId, strlen(param_productId));
  strncat(request_url, product_id, productIdLen);
  strncat(request_url, param_length, strlen(param_length));
  strncat(request_url, string_msgLen, msgLengthLen);
  strncat(request_url, param_period, strlen(param_period));
  strncat(request_url, string_periodLen, periodLen);
  strncat(request_url, param_interval, strlen(param_interval));
  strncat(request_url, string_intervalLen, intervalLen);
  
  mvd_cloud_test_log("request: [%.*s]", request_url_len, request_url);
  
  httpHeader = HTTPHeaderCreate();
  require_action( httpHeader, exit, err = kNoMemoryErr );
  HTTPHeaderClear( httpHeader );
  
  //create tcp connect
  mvd_cloud_test_log("tcp client start to connect...");
  err = gethostbyname((char *)host, (uint8_t *)ipstr, 16);
  require_noerr(err, exit);
  mvd_cloud_test_log("cloud service host:%s, ip: %s", host, ipstr);
  
  tcpClient_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  require(tcpClient_fd != -1, exit);
  
  addr.s_ip = inet_addr(ipstr); 
  addr.s_port = port;
  
  err = connect(tcpClient_fd, &addr, sizeof(addr));
  require_noerr_quiet(err, exit);
  
  mvd_cloud_test_log("EasyCloud server connected at port=%d, fd=%d", 
                         port,
                         tcpClient_fd);
  
  // send request data
  mvd_cloud_test_log("tcp client send activate request...");
  err = CreateHTTPMessageEx(kHTTPGetMethod, 
                            host, request_url,
                            kMIMEType_JSON, 
                            NULL, 0,
                            &httpRequestData, &httpRequestDataLen);
  require_noerr( err, exit );
  // fix http request data
  memset(httpRequestData, 0, httpRequestDataLen);
  //sprintf(httpRequestData, "GET %s HTTP/1.1\r\nHost:api.easylink.io\r\nCache-Control: no-cache\r\n\r\n", request_url);
  sprintf(httpRequestData, "GET %s HTTP/1.1\r\nHost:api.easylink.io\r\n\r\n", request_url);
  ///////////////////////////////////////////////////
  
  mvd_cloud_test_log("send http package: len=%d,\r\n%s", httpRequestDataLen, httpRequestData);
  
  err = SocketSend( tcpClient_fd, httpRequestData, httpRequestDataLen );
  if (httpRequestData != NULL) {
    free(httpRequestData);
    httpRequestDataLen = 0;
  }
  require_noerr( err, exit );
  
  // get http response
  FD_ZERO(&readfds);
  FD_SET(tcpClient_fd, &readfds);
  t.tv_sec = 3;
  t.tv_usec = 0;
  err = select(1, &readfds, NULL, NULL, &t);
  require(err >= 1, exit);
  
  if (FD_ISSET(tcpClient_fd, &readfds)) {
    err = SocketReadHTTPHeader( tcpClient_fd, httpHeader );             
    switch ( err )
    {
    case kNoErr:{
      // statusCode check
      if(kStatusOK != httpHeader->statusCode){
        mvd_cloud_test_log("ERROR: server response statusCode=%d", 
                              httpHeader->statusCode);
        err = kRequestErr;
        goto exit;
      }
      
      mvd_cloud_test_log("startTest done!");
      err = kNoErr;
      break;
    }
    
    default:{
      mvd_cloud_test_log("ERROR: HTTP Header parse internal error: %d", err);
      break;
    }
    }    
  }

exit:
  mvd_cloud_test_log("Exit: EasyCloud tcp client exit err = %d", err);
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(tcpClient_fd != -1){
    close(tcpClient_fd);
    tcpClient_fd = -1;
  }
  if(NULL != request_url){
    free(request_url);
    request_url = NULL;
  }
  
  return err;
}

OSStatus MVDCloudTest_StopRecv(mico_Context_t* const context)
{
  OSStatus err = kUnknownErr;
  err = kNoErr;
  return err;
}



/**
******************************************************************************
* @file    MVDCloudInterfaces.h 
* @author  Eshen Wang
* @version V0.2.0
* @date    21-Nov-2014
* @brief   This header contains the cloud service interfaces 
*          for MICO virtual device. 
  operation
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


#ifndef __MICO_MVDCLOUDINTERFACES_H_
#define __MICO_MVDCLOUDINTERFACES_H_


#include "MICODefine.h"
#include "EasyCloudServiceDef.h"

/*******************************************************************************
 * DEFINES
 ******************************************************************************/
/* data of get file from server */
typedef struct _dev_get_file_data_t {
  char                   file_path[MAX_SIZE_FILE_PATH];
  char                   file_checksum[MAX_SIZE_FILE_MD5];
  char                   file_version[MAX_SIZE_FW_VERSION];
} MVDGetFileRequestData_t;

/*******************************************************************************
 * INTERFACES
 ******************************************************************************/

//common interfaces
OSStatus MVDCloudInterfaceInit(mico_Context_t* const inContext);
OSStatus MVDCloudInterfaceSend(unsigned char *inBuf, unsigned int inBufLen);
OSStatus MVDCloudInterfaceSendto(const char* topic, 
                                 unsigned char *inBuf, unsigned int inBufLen);

// cloud specifical interfaces
OSStatus MVDCloudInterfaceDevActivate(mico_Context_t* const inContext,
                                      MVDActivateRequestData_t devActivateReqData);
OSStatus MVDCloudInterfaceDevAuthorize(mico_Context_t* const inContext,
                                       MVDAuthorizeRequestData_t devAuthorizeReqData);

OSStatus MVDCloudInterfaceDevFirmwareUpdate(mico_Context_t* const inContext,
                                            MVDOTARequestData_t devOTARequestData);
OSStatus MVDCloudInterfaceResetCloudDevInfo(mico_Context_t* const inContext,
                                            MVDResetRequestData_t devResetRequestData);

OSStatus MVDCloudInterfaceGetFile(mico_Context_t* const inContext,
                                  MVDGetFileRequestData_t devGetFileRequestData);

#endif

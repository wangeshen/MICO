/**
******************************************************************************
* @file    EasyCloudServiceVersion.c
* @author  Eshen Wang
* @version V0.1.0
* @date    21-Nov-2014
* @brief   This file contains the release version of the Easycloud service 
           library based on MICO platform. 
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

#include "EasyCloudService.h"

//#define  debug_out 

//#ifdef debug_out
//#define  _debug_out debug_out
//#else
#define _debug_out(format, ...) do {;}while(0)

#define easycloud_service_version_log(M, ...) custom_log("EasyCloudService", M, ##__VA_ARGS__)
#define easycloud_service_version_log_trace() custom_log_trace("EasyCloudService")
//#endif

/*******************************************************************************
 * DEFINES
 ******************************************************************************/
#define EASYCLOUD_SERVCIE_VERSION_MAIN        0x00
#define EASYCLOUD_SERVCIE_VERSION_SUB         0x01
#define EASYCLOUD_SERVCIE_VERSION_REV         0x00

#define EASYCLOUD_SERVCIE_VERSION             (EASYCLOUD_SERVCIE_VERSION_MAIN << 16 | \
                                                EASYCLOUD_SERVCIE_VERSION_SUB << 8 | \
                                                EASYCLOUD_SERVCIE_VERSION_REV)

/*******************************************************************************
 * IMPLEMENTATIONS
 ******************************************************************************/

OSStatus EasyCloudServiceVersion(int *version)
{
  *version = (int)EASYCLOUD_SERVCIE_VERSION;
  return kNoErr;
}
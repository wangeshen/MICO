/**
******************************************************************************
* @file    EasyCloudService.h 
* @author  Eshen Wang
* @version V0.2.0
* @date    23-Nov-2014
* @brief   This header contains interfaces of EasyCloud service.
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

#ifndef __EASYCLOUD_SERVICE_H_
#define __EASYCLOUD_SERVICE_H_

//#include "MICODefine.h"
#include "EasyCloudServiceDef.h"

/*******************************************************************************
 * INTERFACES
 ******************************************************************************/

OSStatus EasyCloudServiceInit(easycloud_service_context_t* const context);
OSStatus EasyCloudServiceStart(easycloud_service_context_t* const context);  //start thread

EasycCloudServiceState EasyCloudServiceState(easycloud_service_context_t* const context);

OSStatus EasyCloudActivate(easycloud_service_context_t* const context);
OSStatus EasyCloudAuthorize(easycloud_service_context_t* const context);

OSStatus EasyCloudUpload(easycloud_service_context_t* const context, const unsigned char *msg, unsigned int msgLen);

//OSStatus EasyCloudFirmwareUpdate(easycloud_service_context_t* const context); //get rom data
OSStatus EasyCloudGetLatestRomVersion(easycloud_service_context_t* const context); //get rom version
OSStatus EasyCloudGetRomData(easycloud_service_context_t* const context); //get rom data

OSStatus EasyCloudServiceStop(easycloud_service_context_t* const context);
OSStatus EasyCloudServiceDeInit(easycloud_service_context_t* const context);

int EasyCloudServiceVersion(easycloud_service_context_t* const context);

#endif

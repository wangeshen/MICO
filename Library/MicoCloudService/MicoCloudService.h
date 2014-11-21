/**
******************************************************************************
* @file    MicoCloudService.h 
* @author  Eshen Wang
* @version V1.0.0
* @date    15-Oct-2014
* @brief   This header contains function prototypes of cloud service based
           on MICO platform. 
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

#ifndef __MICO_CLOUD_SERVICE_H_
#define __MICO_CLOUD_SERVICE_H_

#include "Common.h"
#include "MicoCloudServiceDef.h"

/*******************************************************************************
 * INTERFACES
 ******************************************************************************/

//old interfaces
void MicoCloudServiceInit(cloud_service_config_t init);
OSStatus MicoCloudServiceStart(void);
OSStatus MicoCloudServiceStop(void);

OSStatus MicoCloudServiceUpload(const unsigned char *msg, unsigned int msglen);
cloudServiceState MicoCloudServiceState(void);

int MicoCloudServiceVersion(void);


#endif

/**
******************************************************************************
* @file    user_params_storage.h
* @author  Eshen Wang
* @version V1.0.0
* @date    22-Apr-2015
* @brief   user params storage function.
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

#ifndef __USER_PARAMS_STORAGE_H_
#define __USER_PARAMS_STORAGE_H_

#include "MICODefine.h"
#include "user_properties.h"

/*******************************************************************************
* FUNCTIONS
******************************************************************************/

// restore user params
OSStatus userParams_RestoreDefault(mico_Context_t *mico_context, user_context_t *user_context);
// read user params from flash
OSStatus userParams_Read(mico_Context_t *mico_context, user_context_t *user_context);
// write user config back into flash
OSStatus userParams_Update(mico_Context_t *mico_context, user_context_t *user_context);


#endif // __USER_PARAMS_STORAGE_H_

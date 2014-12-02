/*******************************************************************************
* @file    EasyCloudUtils.c 
* @author  Eshen.Wang
* @version V1.0.0
* @date    29-Nov-2014
* @brief   These functions support EasyCloud.
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


#include "MICO.h"
#include "StringUtils.h"
#include "EasyCloudUtils.h"


#define kCRLFNewLine     "\r\n"
#define kCRLFLineEnding  "\r\n\r\n"

#define easycloud_utils_log(M, ...) custom_log("EasyCloudUtils", M, ##__VA_ARGS__)

#ifdef MICO_FLASH_FOR_UPDATE
static volatile uint32_t flashStorageAddress = UPDATE_START_ADDRESS;
#endif

// OTA test by WES
unsigned int rom_wrote_size = 0;
md5_context md5;
//unsigned char *md5_input = NULL;
unsigned char md5_16[16] = {0};
char *pmd5_32 = NULL;
char rom_file_md5[32] = {0};

OSStatus CreateHTTPMessageEx( const char *methold, const char * host, 
                             const char *url, const char *contentType, 
                             uint8_t *inData, size_t inDataLen, 
                             uint8_t **outMessage, size_t *outMessageSize )
{
  uint8_t *endOfHTTPHeader;
  OSStatus err = kParamErr;

  require( contentType, exit );
  //require( inData, exit );
  //require( inDataLen, exit );

  err = kNoMemoryErr;
  *outMessage = malloc( inDataLen + 500 );
  require( *outMessage, exit );
  memset(*outMessage, 0, inDataLen + 500);

  // Create HTTP Response
  sprintf( (char*)*outMessage,
          "%s %s %s %s%s %s%s%s %s%s%s %d%s",
          methold, url, "HTTP/1.1", kCRLFNewLine,
          "Host:", host, kCRLFNewLine,
          "Content-Type:", contentType, kCRLFNewLine,
          "Content-Length:", (int)inDataLen, kCRLFLineEnding );

  // outMessageSize will be the length of the HTTP Header plus the data length
  *outMessageSize = strlen( (char*)*outMessage ) + inDataLen;

  endOfHTTPHeader = *outMessage + strlen( (char*)*outMessage );
  if ((NULL != inData) && (inDataLen > 0)){
    memcpy( endOfHTTPHeader, inData, inDataLen );
  }
  err = kNoErr;

exit:
  return err;
}

OSStatus CreateSimpleHTTPFailedMessage( uint8_t **outMessage, size_t *outMessageSize )
{
  OSStatus err = kNoMemoryErr;
  *outMessage = malloc( 200 );
  require( *outMessage, exit );
  
  sprintf( (char*)*outMessage,
          "%s %s %s%s",
          "HTTP/1.1", "500", "FAILED", kCRLFLineEnding );
  *outMessageSize = strlen( (char*)*outMessage );
  
  err = kNoErr;
  
exit:
  return err;
}

int SocketReadHTTPHeaderEx( int inSock, HTTPHeader_t *inHeader )
{
  int        err =0;
  char *          buf;
  char *          dst;
  char *          lim;
  char *          end;
  size_t          len;
  ssize_t         n;
  const char *    value;
  size_t          valueSize;
  
  buf = inHeader->buf;
  dst = buf + inHeader->len;
  lim = buf + sizeof( inHeader->buf );
  for( ;; )
  {
    if(findHeader( inHeader,  &end ))
      break ;
    n = read( inSock, dst, (size_t)( lim - dst ) );
    if(      n  > 0 ) len = (size_t) n;
    else  { err = kConnectionErr; goto exit; }
    dst += len;
    inHeader->len += len;
  }
  
  inHeader->len = (size_t)( end - buf );
  err = HTTPHeaderParse( inHeader );
  require_noerr( err, exit );
  inHeader->extraDataLen = (size_t)( dst - end );
  if(inHeader->extraDataPtr) {
    free((uint8_t *)inHeader->extraDataPtr);
    inHeader->extraDataPtr = 0;
  }
  
  if(inHeader->otaDataPtr) {
    free((uint8_t *)inHeader->otaDataPtr);
    inHeader->otaDataPtr = 0;
  }
  
  /* For MXCHIP OTA function, store extra data to OTA data temporary */
  err = HTTPGetHeaderField( inHeader->buf, inHeader->len, "Content-Type", NULL, NULL, &value, &valueSize, NULL );

  if(err == kNoErr && ((strnicmpx( value, valueSize, kMIMEType_MXCHIP_OTA ) == 0)
                       || (strnicmpx( value, valueSize, kMIMEType_EASYCLOUD_OTA ) == 0))){
#ifdef MICO_FLASH_FOR_UPDATE  
    easycloud_utils_log("Receive OTA data!");        
    err = MicoFlashInitialize( MICO_FLASH_FOR_UPDATE );
    require_noerr(err, exit);
    err = MicoFlashWrite(MICO_FLASH_FOR_UPDATE, &flashStorageAddress, (uint8_t *)end, inHeader->extraDataLen);
    require_noerr(err, exit);
    
    // OTA test by WES
    rom_wrote_size = inHeader->extraDataLen;
    easycloud_utils_log("[total=%d][Header]Flash writed[%d]:\r\n%*.s", rom_wrote_size,
                   inHeader->extraDataLen, inHeader->extraDataLen, (uint8_t *)end);  
    // update MD5
    InitMd5(&md5);
    Md5Update(&md5, (uint8_t *)end, inHeader->extraDataLen);
    
#else
    easycloud_utils_log("OTA flash memory is not existed!");
    err = kUnsupportedErr;
#endif
    goto exit;
  }

  /* For chunked extra data without content length */
  if(inHeader->chunkedData == true){
    inHeader->chunkedDataBufferLen = (inHeader->extraDataLen > 256)? inHeader->extraDataLen:256;
    inHeader->chunkedDataBufferPtr = calloc(inHeader->chunkedDataBufferLen, sizeof(uint8_t)); //Make extra data buffer larger than chunk length
    require_action(inHeader->chunkedDataBufferPtr, exit, err = kNoMemoryErr);
    memcpy((uint8_t *)inHeader->chunkedDataBufferPtr, end, inHeader->extraDataLen);
    inHeader->extraDataPtr = inHeader->chunkedDataBufferPtr;
    return kNoErr;
  }

  /* Extra data with content length */
  if (inHeader->contentLength != 0){ //Content length >0, create a memory buffer (Content length) and store extra data
    size_t copyDataLen = (inHeader->contentLength >= inHeader->extraDataLen)? inHeader->contentLength:inHeader->extraDataLen;
    inHeader->extraDataPtr = calloc(copyDataLen , sizeof(uint8_t));
    require_action(inHeader->extraDataPtr, exit, err = kNoMemoryErr);
    memcpy((uint8_t *)inHeader->extraDataPtr, end, copyDataLen);
    err = kNoErr;
  } /* Extra data without content length, data is ended by conntection close */
  else if(inHeader->extraDataLen != 0){ //Content length =0, but extra data length >0, create a memory buffer (1500)and store extra data
    inHeader->dataEndedbyClose = true;
    inHeader->extraDataPtr = calloc(1500, sizeof(uint8_t));
    require_action(inHeader->extraDataPtr, exit, err = kNoMemoryErr);
    memcpy((uint8_t *)inHeader->extraDataPtr, end, inHeader->extraDataLen);
    err = kNoErr;
  }
  else
    return kNoErr;
  
exit:
  return err;
}

OSStatus SocketReadHTTPBodyEx( int inSock, HTTPHeader_t *inHeader )
{
  OSStatus err = kParamErr;
  ssize_t readResult;
  int selectResult;
  fd_set readSet;
  const char *    value;
  size_t          valueSize;
  size_t    lastChunkLen, chunckheaderLen; 
  char *nextPackagePtr;
#ifdef MICO_FLASH_FOR_UPDATE
  bool writeToFlash = false;
#endif
  
  require( inHeader, exit );
  
  err = kNotReadableErr;
  
  FD_ZERO( &readSet );
  FD_SET( inSock, &readSet );

  /* Chunked data, return after receive one chunk */
  if( inHeader->chunkedData == true ){
    /* Move next chunk to chunked data buffer header point */
    lastChunkLen = inHeader->extraDataPtr - inHeader->chunkedDataBufferPtr + inHeader->contentLength;
    if(inHeader->contentLength) lastChunkLen+=2;  //Last chunck data has a CRLF tail
    memmove( inHeader->chunkedDataBufferPtr, inHeader->chunkedDataBufferPtr + lastChunkLen, inHeader->chunkedDataBufferLen - lastChunkLen  );
    inHeader->extraDataLen -= lastChunkLen;

    while ( findChunkedDataLength( inHeader->chunkedDataBufferPtr, inHeader->extraDataLen, &inHeader->extraDataPtr ,"%llu", &inHeader->contentLength ) == false){
      require_action(inHeader->extraDataLen < inHeader->chunkedDataBufferLen, exit, err=kMalformedErr );

      selectResult = select( inSock + 1, &readSet, NULL, NULL, NULL );
      require( selectResult >= 1, exit ); 

      readResult = read( inSock, inHeader->extraDataPtr, (size_t)( inHeader->chunkedDataBufferLen - inHeader->extraDataLen ) );

      if( readResult  > 0 ) inHeader->extraDataLen += readResult;
      else { err = kConnectionErr; goto exit; }
    }

    chunckheaderLen = inHeader->extraDataPtr - inHeader->chunkedDataBufferPtr;

    if(inHeader->contentLength == 0){ //This is the last chunk
      while( findCRLF( inHeader->extraDataPtr, inHeader->extraDataLen - chunckheaderLen, &nextPackagePtr ) == false){ //find CRLF
        selectResult = select( inSock + 1, &readSet, NULL, NULL, NULL );
        require( selectResult >= 1, exit ); 

        readResult = read( inSock,
                          (uint8_t *)( inHeader->extraDataPtr + inHeader->extraDataLen - chunckheaderLen ),
                          256 - inHeader->extraDataLen ); //Assume chunk trailer length is less than 256 (256 is the min chunk buffer, maybe dangerous

        if( readResult  > 0 ) inHeader->extraDataLen += readResult;
        else { err = kConnectionErr; goto exit; }
      }

      err = kNoErr;
      goto exit;


    }
    else{
      /* Extend chunked data buffer */
      if( inHeader->chunkedDataBufferLen < inHeader->contentLength + chunckheaderLen + 2){
        inHeader->chunkedDataBufferLen = inHeader->contentLength + chunckheaderLen + 256;
        inHeader->chunkedDataBufferPtr = realloc(inHeader->chunkedDataBufferPtr, inHeader->chunkedDataBufferLen);
        require_action(inHeader->extraDataPtr, exit, err = kNoMemoryErr);
      }

      /* Read chunked data */
      while ( inHeader->extraDataLen < inHeader->contentLength + chunckheaderLen + 2 ){
        selectResult = select( inSock + 1, &readSet, NULL, NULL, NULL );
        require( selectResult >= 1, exit ); 

        readResult = read( inSock,
                          (uint8_t *)( inHeader->extraDataPtr + inHeader->extraDataLen - chunckheaderLen),
                          ( inHeader->contentLength - (inHeader->extraDataLen - chunckheaderLen) + 2 ));
        
        if( readResult  > 0 ) inHeader->extraDataLen += readResult;
        else { err = kConnectionErr; goto exit; }
      } 
      
      if( *(inHeader->extraDataPtr + inHeader->contentLength) != '\r' ||
         *(inHeader->extraDataPtr + inHeader->contentLength +1 ) != '\n'){
           err = kMalformedErr; 
           goto exit;
         }
    }
  }

  /* We has extra data but total length is not clear, store them to 1500 bytes buffer 
     return when connection is disconnected by remote server */
  if( inHeader->dataEndedbyClose == true){ 
    if(inHeader->contentLength == 0) { //First read body, return using data received by SocketReadHTTPHeader
      inHeader->contentLength = inHeader->extraDataLen;
    }else{
      selectResult = select( inSock + 1, &readSet, NULL, NULL, NULL );
      require( selectResult >= 1, exit ); 
      
      readResult = read( inSock,
                        (uint8_t*)( inHeader->extraDataPtr ),
                        1500 );
      if( readResult  > 0 ) inHeader->contentLength = readResult;
      else { err = kConnectionErr; goto exit; }
    }
    err = kNoErr;
    goto exit;
  }
  
  /* We has extra data and we has a predefined buffer to store the total extra data
     return when all data has received*/
  while ( inHeader->extraDataLen < inHeader->contentLength )
  {
    selectResult = select( inSock + 1, &readSet, NULL, NULL, NULL );
    require( selectResult >= 1, exit );
    
    
    err = HTTPGetHeaderField( inHeader->buf, inHeader->len, "Content-Type", NULL, NULL, &value, &valueSize, NULL );
    require_noerr(err, exit);
    if( (strnicmpx( value, valueSize, kMIMEType_MXCHIP_OTA ) == 0) 
       || (strnicmpx( value, valueSize, kMIMEType_EASYCLOUD_OTA ) == 0) ){
#ifdef MICO_FLASH_FOR_UPDATE  
      writeToFlash = true;
      inHeader->otaDataPtr = calloc(OTA_Data_Length_per_read, sizeof(uint8_t)); 
      require_action(inHeader->otaDataPtr, exit, err = kNoMemoryErr);
      if((inHeader->contentLength - inHeader->extraDataLen)<OTA_Data_Length_per_read){
        readResult = read( inSock,
                          (uint8_t*)( inHeader->otaDataPtr ),
                          ( inHeader->contentLength - inHeader->extraDataLen ) );
      }else{
        readResult = read( inSock,
                          (uint8_t*)( inHeader->otaDataPtr ),
                          OTA_Data_Length_per_read);
      }
      
      if( readResult  > 0 ) inHeader->extraDataLen += readResult;
      else { err = kConnectionErr; goto exit; }
      
      err = MicoFlashWrite(MICO_FLASH_FOR_UPDATE, &flashStorageAddress, (uint8_t *)inHeader->otaDataPtr, readResult);
      require_noerr(err, exit);
         
      // OTA test by WES
      rom_wrote_size += readResult;
      easycloud_utils_log("[total=%d][Body]Flash writed[%d]:\r\n%*.s", rom_wrote_size,
                   readResult, readResult, (uint8_t *)inHeader->otaDataPtr);
      
      // update MD5
      Md5Update(&md5, (uint8_t *)inHeader->otaDataPtr, readResult);
      
      free(inHeader->otaDataPtr);
      inHeader->otaDataPtr = 0;
#else
      easycloud_utils_log("OTA flash memory is not existed, !");
      err = kUnsupportedErr;
#endif
    }else{
      readResult = read( inSock,
                        (uint8_t*)( inHeader->extraDataPtr + inHeader->extraDataLen ),
                        ( inHeader->contentLength - inHeader->extraDataLen ) );
      
      if( readResult  > 0 ) inHeader->extraDataLen += readResult;
      else { err = kConnectionErr; goto exit; }
    }
  }
  // recv done, check MD5
  Md5Final(&md5, md5_16);
  //convert hex data to hex string
  pmd5_32 = DataToHexStringLowercase(md5_16,  sizeof(md5_16));
  easycloud_utils_log("final_md5[%d]=%s", strlen(pmd5_32), pmd5_32);
  
  if (NULL != pmd5_32){
    strncpy(rom_file_md5, pmd5_32, strlen(pmd5_32));
    free(pmd5_32);
    pmd5_32 = NULL;
  }
  else
    return kNoMemoryErr;
  
  err = kNoErr;
  
exit:
  if(err != kNoErr) inHeader->len = 0;
  if(inHeader->otaDataPtr) {
    free(inHeader->otaDataPtr);
    inHeader->otaDataPtr = 0;
  }
#ifdef MICO_FLASH_FOR_UPDATE
  if(writeToFlash == true) MicoFlashFinalize(MICO_FLASH_FOR_UPDATE);
#endif
  return err;
}


char* DataToHexStringLowercase( const uint8_t *inBuf, size_t inBufLen )
{
    char* buf_str = NULL;
    char* buf_ptr;
    require_quiet(inBuf, error);
    require_quiet(inBufLen > 0, error);

    buf_str = (char*) malloc (2*inBufLen + 1);
    require(buf_str, error);
    buf_ptr = buf_str;
    uint32_t i;
    for (i = 0; i < inBufLen; i++) buf_ptr += sprintf(buf_ptr, "%02x", inBuf[i]);
    *buf_ptr = '\0';
    return buf_str;

error:
    if ( buf_str ) free( buf_str );
    return NULL;
}


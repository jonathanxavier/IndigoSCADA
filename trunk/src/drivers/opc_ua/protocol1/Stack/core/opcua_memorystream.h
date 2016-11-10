/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _OpcUa_MemoryStream_H_
#define _OpcUa_MemoryStream_H_ 1
#ifdef OPCUA_HAVE_MEMORYSTREAM
#include <opcua_stream.h>

OPCUA_BEGIN_EXTERN_C
 
/** 
  @brief Allocates a new readable memory stream.
 
  The caller must ensure the buffer is valid memory until Close is called.

  @param buffer     [in]  The buffer which is the source for the stream.
  @param bufferSize [in]  The length of the buffer.
  @param istrm      [out] The input stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_MemoryStream_CreateReadable(
    OpcUa_Byte*         buffer,
    OpcUa_UInt32        bufferSize, 
    OpcUa_InputStream** istrm);

/** 
  @brief Allocates a new writeable memory stream.
 
  @param blockSize  [in]  The size of the block to allocate when new memory is required.
  @param maxSize    [in]  The maximum buffer size (0 means no limit).
  @param ostrm      [out] The output stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_MemoryStream_CreateWriteable(
    OpcUa_UInt32         blockSize,
    OpcUa_UInt32         maxSize,
    OpcUa_OutputStream** ostrm);

/** 
  @brief Returns the internal buffer for a writeable stream.
 
  This function cannot be called until the stream is closed.

  The memory returned by this function is owned by the stream and will be
  de-allocated when OpcUa_MemoryStream_Delete is called.

  @param ostrm      [in]  The output stream.
  @param buffer     [out] The buffer which contains the data written to the stream.
  @param bufferSize [out] The amount of valid data in the buffer.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_MemoryStream_GetBuffer(
    OpcUa_OutputStream* ostrm,
    OpcUa_Byte**        buffer,
    OpcUa_UInt32*       bufferSize);

OPCUA_END_EXTERN_C

#endif /* OPCUA_HAVE_MEMORYSTREAM */
#endif /* _OpcUa_MemoryStream_H_ */

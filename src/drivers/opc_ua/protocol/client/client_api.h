/* ========================================================================
* Copyright (c) 2005-2016 The OPC Foundation, Inc. All rights reserved.
*
* OPC Foundation MIT License 1.00
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* The complete license agreement can be found here:
* http://opcfoundation.org/License/MIT/1.00/
* ======================================================================*/
//#pragma once

#ifndef OPCUA_CLIENT_API
#define OPCUA_CLIENT_API

/*********************************************************************************************/
/***********************                     Config                   ************************/
/*********************************************************************************************/
/* !0 == active, 0 == inactive */

/* client signs the messages */
#define UACLIENT_USE_SIGNING                            0
/* client encrypts the messages */
#define UACLIENT_USE_ENCRYPTION                         0
/* use the synchronous api - only possible when multithreading is supported */
#define UACLIENT_USE_SYNC_API                           1
/* standard timeout for connect process */
#define UACLIENT_TIMEOUT                                OPCUA_INFINITE
/* URL of the server */
//#define UACLIENT_SERVER_URL                             "opc.tcp://localhost:4840"
//#define UACLIENT_SERVER_URL                             "opc.tcp://opcuaserver.com:48010"

/* Transport profile used by the client */
#define UACLIENT_TRANSPORT_PROFILE_URI                  OpcUa_TransportProfile_UaTcp
/* the used trace level */
//#define UACLIENT_TRACE_LEVEL                            OPCUA_TRACE_OUTPUT_LEVEL_SYSTEM
#define UACLIENT_TRACE_LEVEL                            OPCUA_TRACE_OUTPUT_LEVEL_CONTENT

/* whether to wait for user input */
#define UACLIENT_WAIT_FOR_USER_INPUT                    1

/* PKI Locations - this may need to be changed with other platform layers! */
#define UACLIENT_CERTIFICATE_TRUST_LIST_LOCATION                "../PKI/certs/"
#define UACLIENT_CERTIFICATE_REVOCATION_LIST_LOCATION           "../PKI/crl/"
#define UACLIENT_CERTIFICATE_LOCATION                           "../PKI/certs/Opc.Ua.Publisher [A482BEA2CEC01DCE670A6624A131943D3B1020AC].der"
#define UACLIENT_PRIVATE_KEY_LOCATION                           "../PKI/private/Opc.Ua.Publisher [A482BEA2CEC01DCE670A6624A131943D3B1020AC].pfx"

#define UACLIENTNAME              "Opc.Ua.Client"
#define UACLIENTURI               "urn:localhost:OPCFoundation:AMQPPublisher"
#define UACLIENT_PRODUCTURI		  "http://opcfoundation.org/UA/Publisher/"

/* configuration checks */
#if UACLIENT_USE_SIGNING || UACLIENT_USE_ENCRYPTION
#define UACLIENT_USE_SECURE_COMMUNICATION               1
#else
#define UACLIENT_USE_SECURE_COMMUNICATION               0
#endif

#if UACLIENT_USE_ENCRYPTION
#if UACLIENT_USE_SIGNING
/* encryption always includes signing - undef for evaluation only */
#undef UACLIENT_USE_SIGNING
#endif
#endif

#if UACLIENT_USE_SECURE_COMMUNICATION
/* verify the servers certificate after loading, before connecting */
#define UACLIENT_VERIFY_SERVER_CERTIFICATE_LOCALLY  1
/* verify the clients certificate after loading, before connecting */
#define UACLIENT_VERIFY_CLIENT_CERTIFICATE_LOCALLY  1
#endif

/* switch between security policies based on above configuration */
#define UACLIENT_SECURITY_POLICY_NONE               OpcUa_SecurityPolicy_None

#if UACLIENT_USE_ENCRYPTION
#define UACLIENT_SECURITY_POLICY                    OpcUa_SecurityPolicy_Basic128Rsa15
#define UACLIENT_SECURITY_POLICY_LENGTH             OpcUa_StrLenA(OpcUa_SecurityPolicy_Basic128Rsa15)
#define UACLIENT_SECURITY_MODE                      OpcUa_MessageSecurityMode_SignAndEncrypt
#endif

#if UACLIENT_USE_SIGNING
#define UACLIENT_SECURITY_POLICY                    OpcUa_SecurityPolicy_Basic256
#define UACLIENT_SECURITY_POLICY_LENGTH             OpcUa_StrLenA(OpcUa_SecurityPolicy_Basic256)
#define UACLIENT_SECURITY_MODE                      OpcUa_MessageSecurityMode_Sign
#endif

#if !UACLIENT_USE_ENCRYPTION && !UACLIENT_USE_SIGNING
#define UACLIENT_SECURITY_POLICY                    OpcUa_SecurityPolicy_None
#define UACLIENT_SECURITY_POLICY_LENGTH             OpcUa_StrLenA(OpcUa_SecurityPolicy_None)
#define UACLIENT_SECURITY_MODE                      OpcUa_MessageSecurityMode_None
#endif

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include <opcua_clientproxy.h>
#include <opcua_memory.h>
#include <opcua_core.h>
#include <opcua_trace.h>
#include <opcua_string.h>
#include <opcua_endpoint.h>
#include <opcua_datetime.h>

#if OPCUA_HAVE_HTTPS
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

typedef struct _Session
{
	OpcUa_Channel Channel;
	OpcUa_String EndpointUrl;
	OpcUa_String ApplicationUri;
	OpcUa_NodeId SessionId;
	OpcUa_NodeId AuthenticationToken;
	OpcUa_Double RevisedSessionTimeout;
	OpcUa_ByteString ServerNonce;
	OpcUa_ByteString ServerCertificate;
	OpcUa_UserTokenPolicy IdentityTokenPolicy;
	OpcUa_ExtensionObject IdentityToken;
	OpcUa_UInt32 SubscriptionId;
	OpcUa_Int32 SequenceNumbersCount;
	OpcUa_UInt32* SequenceNumbers;
	OpcUa_Int32 MonitoredItemsCount;
	OpcUa_UInt32* MonitoredItems;
	OpcUa_DataValue* MonitoredValues;
}
Session;


#ifdef  __cplusplus
extern "C" {
#endif

OpcUa_StatusCode Client_Initialize(OpcUa_Void);
OpcUa_Void Client_SecurityClear(OpcUa_Void);
OpcUa_Void Client_Cleanup(OpcUa_Void);
void Session_Initialize(Session* pSession);
void Session_Clear(Session* pSession);
OpcUa_StatusCode Client_Connect(Session* a_pSession, OpcUa_Boolean a_bUseSecurity, char* server_url);
void* OpcUa_ExtensionObject_CreateFromType(OpcUa_ExtensionObject* a_pExtension, OpcUa_EncodeableType* a_pType);
OpcUa_StatusCode Client_SelectAnonymousUserTokenPolicy(Session* a_pSession, OpcUa_EndpointDescription* a_pEndpoint);
OpcUa_StatusCode Client_GetEndpoints(Session* a_pSession, char* server_url);
OpcUa_StatusCode Client_CreateSession(Session* a_pSession);
OpcUa_StatusCode Client_ActivateSession(Session* a_pSession);
OpcUa_StatusCode Client_Browse(Session* a_pSession, OpcUa_Int a_iNodeID);
OpcUa_StatusCode Client_ReadNode(Session* a_pSession, char* node_id, int ns_idx, OpcUa_DataValue** pValueRead);
OpcUa_StatusCode Client_CloseSession(Session* a_pSession);

#ifdef  __cplusplus
}
#endif

#endif //OPCUA_CLIENT_API
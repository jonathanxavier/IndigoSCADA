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

#ifndef _OpcUa_P_OpenSSL_PKI_H_
#define _OpcUa_P_OpenSSL_PKI_H_ 1

OPCUA_BEGIN_EXTERN_C

/** 
  @brief Creates a certificate store object.
 
  @param pProvider                  [in]  The crypto provider handle.
  
  @param ppCertificateStore         [out] The handle to the certificate store. Type depends on store implementation.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_OpenCertificateStore(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_Void**                ppCertificateStore);

/** 
  @brief frees a certificate store object.

  @param pProvider             [in]  The crypto provider handle.
  @param ppCertificateStore    [in] The certificate store object. Type depends on store implementation.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_CloseCertificateStore(
    OpcUa_PKIProvider*       pProvider,
    OpcUa_Void**             ppCertificateStore);

/** 
  @brief Validates a given X509 certificate object.
 
   Validation:
   - Subject/Issuer
   - Path
   - Certificate Revocation List (CRL)
   - Certificate Trust List (CTL)

  @param pProvider                [in]  The crypto provider handle.
  @param pCertificate             [in]  The certificate that should be validated.
  @param pCertificateStore        [in]  The certificate store that validates the passed in certificate.

  @param pValidationCode          [out] The validation code, that gives information about the validation result. Validation return codes from OpenSSL are used.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_ValidateCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Int*                  pValidationCode);

/** 
  @brief imports a given certificate into given certificate store.
 
  @param pProvider                [in]  The crypto provider handle.
  @param pCertificateStore        [in]  The certificate store that should store the passed in certificate.
  @param pCertificate             [in]  The certificate that should be stored in the certificate store. 
  @param pSaveHandle              [out]  The handle that indicates the save location of the certificate within then certificate store.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_SaveCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Void*                 pSaveHandle);

/** 
  @brief exports a certain certificate from a given certificate store.
 
  @param pProvider                [in]  The crypto provider handle.
  @param pLoadHandle              [in]  The handle that indicates the load location of the certificate within then certificate store.
  @param ppCertificateStore       [in]  The certificate store that contains the desired certificate.

  @param pCertificate             [out] The desired certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_LoadCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_Void*                 pLoadHandle,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_ByteString*           pCertificate);

/**
  @brief Extracts data from a certificate store object.

  @param pCertificate          [in] The certificate to examine.
  @param pIssuer               [out, optional] The issuer name of the certificate.
  @param pSubject              [out, optional] The subject name of the certificate.
  @param pSubjectUri           [out, optional] The subject's URI of the certificate.
  @param pSubjectIP            [out, optional] The subject's IP of the certificate.
  @param pSubjectDNS           [out, optional] The subject's DNS name of the certificate.
  @param pCertThumbprint       [out, optional] The thumbprint of the certificate.
  @param pSubjectHash          [out, optional] The hash code of the certificate.
  @param pCertRawLength        [out, optional] The length of the DER encoded data.
                               can be smaller than the total length of pCertificate in case of chain certificate or garbage follow.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_ExtractCertificateData(
    OpcUa_ByteString*           pCertificate,
    OpcUa_ByteString*           pIssuer,
    OpcUa_ByteString*           pSubject,
    OpcUa_ByteString*           pSubjectUri,
    OpcUa_ByteString*           pSubjectIP,
    OpcUa_ByteString*           pSubjectDNS,
    OpcUa_ByteString*           pCertThumbprint,
    OpcUa_UInt32*               pSubjectHash,
    OpcUa_UInt32*               pCertRawLength);

/* NoSecurity functions */

/** 
  @brief Creates a certificate store object.
 
  @param pProvider                  [in]  The crypto provider handle.
  
  @param ppCertificateStore         [out] The handle to the certificate store. Type depends on store implementation.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_NoSecurity_OpenCertificateStore(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_Void**                ppCertificateStore);

/** 
  @brief frees a certificate store object.

  @param pProvider             [in]  The crypto provider handle.
  @param ppCertificateStore    [in] The certificate store object. Type depends on store implementation.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_NoSecurity_CloseCertificateStore(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_Void**                ppCertificateStore);

/** 
  @brief Validates a given X509 certificate object.
 
   Validation:
   - Subject/Issuer
   - Path
   - Certificate Revocation List (CRL)
   - Certificate Trust List (CTL)

  @param pProvider                [in]  The crypto provider handle.
  @param pCertificate             [in]  The certificate that should be validated.
  @param pCertificateStore        [in]  The certificate store that validates the passed in certificate.

  @param pValidationCode          [out] The validation code, that gives information about the validation result. Validation return codes from OpenSSL are used.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_NoSecurity_ValidateCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Int*                  pValidationCode /* Validation return codes from OpenSSL */);

/** 
  @brief imports a given certificate into given certificate store.
 
  @param pProvider                [in]  The crypto provider handle.
  @param pCertificateStore        [in]  The certificate store that should store the passed in certificate.
  @param pCertificate             [in]  The certificate that should be stored in the certificate store. 
  @param pSaveHandle              [out]  The handle that indicates the save location of the certificate within then certificate store.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_NoSecurity_SaveCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Void*                 pSaveHandle);

/** 
  @brief exports a certain certificate from a given certificate store.
 
  @param pProvider                [in]  The crypto provider handle.
  @param pLoadHandle              [in]  The handle that indicates the load location of the certificate within then certificate store.
  @param ppCertificateStore       [in]  The certificate store that contains the desired certificate.

  @param pCertificate             [out] The desired certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_NoSecurity_LoadCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_Void*                 pLoadHandle,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_ByteString*           pCertificate);

/** 
  @brief exports a certain certificate from a given certificate store.
 
  @param pProvider                [in]  Load handle of the key file (ie. path in directory based PKI's).
  @param pLoadHandle              [in]  The format in which the key is stored.
  @param ppCertificateStore       [in]  The password if the key file is password secured.

  @param pCertificate             [out] The desired private key.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_NoSecurity_LoadPrivateKeyFromFile(
    OpcUa_StringA               privateKeyFile,
    OpcUa_P_FileFormat          fileFormat,
    OpcUa_StringA               password,
    OpcUa_ByteString*           pPrivateKey);

/**
  @brief Extracts data from a certificate store object.

  @param pCertificate          [in] The certificate to examine.
  @param pIssuer               [out, optional] The issuer name of the certificate.
  @param pSubject              [out, optional] The subject name of the certificate.
  @param pSubjectUri           [out, optional] The subject's URI of the certificate.
  @param pSubjectIP            [out, optional] The subject's IP of the certificate.
  @param pSubjectDNS           [out, optional] The subject's DNS name of the certificate.
  @param pCertThumbprint       [out, optional] The thumbprint of the certificate.
  @param pSubjectHash          [out, optional] The hash code of the certificate.
  @param pCertRawLength        [out, optional] The length of the DER encoded data.
                               can be smaller than the total length of pCertificate in case of chain certificate or garbage follow.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_NoSecurity_ExtractCertificateData(
    OpcUa_ByteString*           pCertificate,
    OpcUa_ByteString*           pIssuer,
    OpcUa_ByteString*           pSubject,
    OpcUa_ByteString*           pSubjectUri,
    OpcUa_ByteString*           pSubjectIP,
    OpcUa_ByteString*           pSubjectDNS,
    OpcUa_ByteString*           pCertThumbprint,
    OpcUa_UInt32*               pSubjectHash,
    OpcUa_UInt32*               pCertRawLength);

OPCUA_END_EXTERN_C

#endif /* _OpcUa_Crypto_OpenSsl_H_ */

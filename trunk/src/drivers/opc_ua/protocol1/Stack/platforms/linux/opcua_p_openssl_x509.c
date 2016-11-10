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

/* UA platform definitions */
#include <opcua_p_internal.h>
#include <opcua_p_datetime.h>
#include <opcua_p_memory.h>
#include <opcua_p_string.h>
#include <opcua_p_guid.h>

#if OPCUA_REQUIRE_OPENSSL

/* System Headers */
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs12.h>
#include <openssl/err.h>


/* own headers */
#include <opcua_p_openssl.h>

/**
  @brief Add a certificate name entry to a X.509 (X509) certificate.

  internal function!

  @param ppX509Name    [out]  The X509 Name Object.

  @param pNameEntry    [in]   The passed Name Entry Object.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_Name_AddEntry(
    X509_NAME               **ppX509Name,
    OpcUa_Crypto_NameEntry *pNameEntry);

/*============================================================================
 * OpcUa_P_OpenSSL_X509_Name_AddEntry
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_Name_AddEntry(
    X509_NAME**               a_ppX509Name,
    OpcUa_Crypto_NameEntry*   a_pNameEntry)
{
    X509_NAME_ENTRY*    pEntry  = OpcUa_Null;

    OpcUa_Int           nid;

    OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "X509_Name_AddEntry");

    OpcUa_ReturnErrorIfArgumentNull(a_pNameEntry);

    if((nid = OBJ_txt2nid(a_pNameEntry->key)) == NID_undef)
    {
        uStatus =  OpcUa_BadNotSupported;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(!(pEntry = X509_NAME_ENTRY_create_by_NID(OpcUa_Null, nid, MBSTRING_ASC, (unsigned char*)a_pNameEntry->value, -1)))
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(X509_NAME_add_entry(*a_ppX509Name, pEntry,-1,0) != 1)
    {
        uStatus =  OpcUa_Bad;
    }

    if(pEntry != OpcUa_Null)
    {
        X509_NAME_ENTRY_free(pEntry);
    }

OpcUa_ReturnStatusCode;

OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_X509_AddCustomExtension
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_AddCustomExtension(
    X509**                   a_ppCertificate,
    OpcUa_Crypto_Extension*  a_pExtension,
    X509V3_CTX*              a_pX509V3Context)
{
    X509_EXTENSION*     pExtension   = OpcUa_Null;
    char*               pName        = OpcUa_Null;
    char*               pValue       = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "X509_AddCustomExtension");

    OpcUa_ReturnErrorIfArgumentNull(a_pX509V3Context);
    OpcUa_ReturnErrorIfArgumentNull(a_pExtension->key);
    OpcUa_ReturnErrorIfArgumentNull(a_pExtension->value);

    pName = (char*)a_pExtension->key;
    pValue = (char*)a_pExtension->value;

    /* create the extension. */
    pExtension = X509V3_EXT_conf(
        OpcUa_Null,
        a_pX509V3Context,
        pName,
        pValue);

    if (pExtension == OpcUa_Null)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_Bad);
    }

    /* add it to the certificate. */
    if (!X509_add_ext(*a_ppCertificate, pExtension, -1))
    {
        OpcUa_GotoErrorWithStatus(OpcUa_Bad);
    }

    /* free the extension. */
    X509_EXTENSION_free(pExtension);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pExtension != OpcUa_Null)
    {
        X509_EXTENSION_free(pExtension);
    }

OpcUa_FinishErrorHandling;
}

/** \internal Compares the two given DateTime values.
 * @return 0 if equal, -1 if \c a is smaller than \c b, 1 otherwise.
 */
int OpcUa_P_DateTime_Compare(const OpcUa_DateTime *a, const OpcUa_DateTime *b)
{
    if (a == b) return 0;
    if (a->dwHighDateTime == b->dwHighDateTime)
    {
        if (a->dwLowDateTime == b->dwLowDateTime) return 0;
        if (a->dwLowDateTime < b->dwLowDateTime) return -1;
        return 1;
    }
    else if (a->dwHighDateTime < b->dwHighDateTime)
    {
        return -1;
    }
    return 1;
}

/*============================================================================
 * OpcUa_P_OpenSSL_X509_SelfSigned_Custom_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_SelfSigned_Custom_Create(
    OpcUa_CryptoProvider*       a_pProvider,
    OpcUa_Int32                 a_serialNumber,
    OpcUa_UInt32                a_validToInSec,
    OpcUa_Crypto_NameEntry*     a_pNameEntries,      /* will be used for issuer and subject thus it's selfigned cert */
    OpcUa_UInt                  a_nameEntriesCount,  /* will be used for issuer and subject thus it's selfigned cert */
    OpcUa_Key                   a_pSubjectPublicKey, /* EVP_PKEY* - type defines also public key algorithm */
    OpcUa_Crypto_Extension*     a_pExtensions,
    OpcUa_UInt                  a_extensionsCount,
    OpcUa_UInt                  a_signatureHashAlgorithm, /* EVP_sha1(),... */
    OpcUa_Key                   a_pIssuerPrivateKey, /* EVP_PKEY* - type defines also signature algorithm */
    OpcUa_ByteString*           a_pCertificate)
{
    OpcUa_UInt          i;

    X509_NAME*          pSubj               = OpcUa_Null;
    X509V3_CTX          ctx;
    const EVP_MD*       pDigest             = OpcUa_Null;

    X509*               pCert               = OpcUa_Null;
    EVP_PKEY*           pSubjectPublicKey   = OpcUa_Null;
    EVP_PKEY*           pIssuerPrivateKey   = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "X509_SelfSigned_Custom_Create");

    OpcUa_ReferenceParameter(a_pProvider);

    OpcUa_ReturnErrorIfArgumentNull(a_pNameEntries);
    OpcUa_ReturnErrorIfArgumentNull(a_pExtensions);
    OpcUa_ReturnErrorIfArgumentNull(a_pIssuerPrivateKey.Key.Data);
    OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);

    if(a_pSubjectPublicKey.Type != OpcUa_Crypto_KeyType_Rsa_Public)
    {
        uStatus =  OpcUa_BadInvalidArgument;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(a_pIssuerPrivateKey.Type != OpcUa_Crypto_KeyType_Rsa_Private)
    {
        uStatus =  OpcUa_BadInvalidArgument;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    pSubjectPublicKey = d2i_PublicKey(EVP_PKEY_RSA,OpcUa_Null,((const unsigned char**)&(a_pSubjectPublicKey.Key.Data)),a_pSubjectPublicKey.Key.Length);
    if (pSubjectPublicKey == OpcUa_Null)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    pIssuerPrivateKey = d2i_PrivateKey(EVP_PKEY_RSA,OpcUa_Null,((const unsigned char**)&(a_pIssuerPrivateKey.Key.Data)),a_pIssuerPrivateKey.Key.Length);
    if (pIssuerPrivateKey == OpcUa_Null)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* create new certificate object */
    pCert = X509_new();
    if (pCert == OpcUa_Null)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* set version of certificate (V3 since internal representation starts versioning from 0) */
    if(X509_set_version(pCert, 2L) != 1)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* generate a unique number for a serial number if none provided. */
    if (a_serialNumber == 0)
    {
        ASN1_INTEGER* pSerialNumber = X509_get_serialNumber(pCert);

        pSerialNumber->type   = V_ASN1_INTEGER;
        pSerialNumber->data   = OPENSSL_realloc(pSerialNumber->data, 16);
        pSerialNumber->length = 16;

        OpcUa_P_Guid_Create((OpcUa_Guid*)pSerialNumber->data);
    }

    /* use the integer passed in - note the API should not be using a 32-bit integer - must fix sometime */
    else if (ASN1_INTEGER_set(X509_get_serialNumber(pCert), a_serialNumber) == 0)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* add key to the request */
    if(X509_set_pubkey(pCert, pSubjectPublicKey) != 1)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(pSubjectPublicKey != OpcUa_Null)
    {
        EVP_PKEY_free(pSubjectPublicKey);
        pSubjectPublicKey = OpcUa_Null;
    }

    /* assign the subject name */
    pSubj = X509_NAME_new();
    if(!pSubj)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* create and add entries to subject name */
    for(i=0; i<a_nameEntriesCount; i++)
    {
        uStatus = OpcUa_P_OpenSSL_X509_Name_AddEntry(&pSubj, a_pNameEntries + i);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* set subject name in request */
    if(X509_set_subject_name(pCert, pSubj) != 1)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* set name of issuer (CA) */
    if(X509_set_issuer_name(pCert, pSubj) != 1)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(!(X509_gmtime_adj(X509_get_notBefore(pCert), 0)))
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* set ending time of the certificate */
    if(!(X509_gmtime_adj(X509_get_notAfter(pCert), a_validToInSec)))
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* add x509v3 extensions */
    X509V3_set_ctx(&ctx, pCert, pCert, OpcUa_Null, OpcUa_Null, 0);

    for(i=0; i<a_extensionsCount; i++)
    {
        uStatus = OpcUa_P_OpenSSL_X509_AddCustomExtension(&pCert, a_pExtensions+i, &ctx);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* sign certificate with the CA private key */
    switch(a_signatureHashAlgorithm)
    {
    case OPCUA_P_SHA_160:
        pDigest = EVP_sha1();
        break;
    case OPCUA_P_SHA_224:
        pDigest = EVP_sha224();
        break;
    case OPCUA_P_SHA_256:
        pDigest = EVP_sha256();
        break;
    case OPCUA_P_SHA_384:
        pDigest = EVP_sha384();
        break;
    case OPCUA_P_SHA_512:
        pDigest = EVP_sha512();
        break;
    default:
        uStatus =  OpcUa_BadNotSupported;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(!(X509_sign(pCert, pIssuerPrivateKey, pDigest)))
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if (X509_verify(pCert, pIssuerPrivateKey) <= 0)
    {
        uStatus =  OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(pIssuerPrivateKey != OpcUa_Null)
    {
        EVP_PKEY_free(pIssuerPrivateKey);
        pIssuerPrivateKey = OpcUa_Null;
    }

    if(pSubj != OpcUa_Null)
    {
        X509_NAME_free(pSubj);
        pSubj = OpcUa_Null;
    }

    /* prepare container */
    memset(a_pCertificate, 0, sizeof(OpcUa_ByteString));

    /* get required length for conversion target buffer */
    a_pCertificate->Length = i2d_X509(pCert, NULL);

    if(a_pCertificate->Length <= 0)
    {
        /* conversion to DER not possible */
        uStatus = OpcUa_Bad;
    }

    /* allocate conversion target buffer */
    a_pCertificate->Data = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(a_pCertificate->Length);
    OpcUa_GotoErrorIfAllocFailed(a_pCertificate->Data);

    /* convert into DER */
    a_pCertificate->Length = i2d_X509(pCert, &(a_pCertificate->Data));
    if(a_pCertificate->Length <= 0)
    {
        /* conversion to DER not possible */
        uStatus = OpcUa_Bad;
    }
    else
    {
        /* correct pointer incrementation by i2d_X509() */
        a_pCertificate->Data -= a_pCertificate->Length;
    }

    X509_free(pCert);

OpcUa_ReturnStatusCode;

OpcUa_BeginErrorHandling;

    X509_free(pCert);

    if(pSubjectPublicKey != OpcUa_Null)
    {
        EVP_PKEY_free(pSubjectPublicKey);
        pSubjectPublicKey = OpcUa_Null;
    }

    if(pIssuerPrivateKey != OpcUa_Null)
    {
        EVP_PKEY_free(pIssuerPrivateKey);
        pIssuerPrivateKey = OpcUa_Null;
    }

    if(pSubj != OpcUa_Null)
    {
        X509_NAME_free(pSubj);
        pSubj = OpcUa_Null;
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_X509_LoadFromFile
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_LoadFromFile(
    OpcUa_StringA               a_fileName,
    OpcUa_P_FileFormat          a_fileFormat,
    OpcUa_StringA               a_sPassword,        /* optional: just for OpcUa_PKCS12 */
    OpcUa_ByteString*           a_pCertificate)
{
    BIO*            pCertFile       = OpcUa_Null;
    X509*           pCertX509       = OpcUa_Null;
    PKCS12*         pPKCS12Cert     = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "X509_LoadFromFile");

    /* check filename */
    if(OpcUa_P_String_strlen(a_fileName) < 1)
    {
        uStatus = OpcUa_BadInvalidArgument;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* import certificate from file by the given encoding type */
    pCertFile = BIO_new_file((const char*)a_fileName, "r");
    OpcUa_ReturnErrorIfArgumentNull(pCertFile);

    switch(a_fileFormat)
    {
    case OpcUa_Crypto_Encoding_DER:
        {

            pCertX509 = d2i_X509_bio(pCertFile,      /* sourcefile */
                                     (X509**)NULL);  /* target (if preallocated) */
            break;
        }
    case OpcUa_Crypto_Encoding_PEM:
        {

            pCertX509 = PEM_read_bio_X509(  pCertFile,          /* sourcefile */
                                            (X509**)OpcUa_Null, /* target (if preallocated) */
                                            OpcUa_Null,         /* password callback function */
                                            OpcUa_Null);        /* passphrase or callback data */
            break;
        }
    case OpcUa_Crypto_Encoding_PKCS12:
        {
            d2i_PKCS12_bio(pCertFile, &pPKCS12Cert);

            PKCS12_parse(pPKCS12Cert, a_sPassword, OpcUa_Null, &pCertX509, OpcUa_Null);

            if(pPKCS12Cert != OpcUa_Null)
            {
                PKCS12_free(pPKCS12Cert);
                /*OPENSSL_free(pPKCS12Cert);*/
            }

            break;
        }
    default:
        {
            BIO_free(pCertFile);
            return OpcUa_BadNotSupported;
        }
    }

    BIO_free(pCertFile);
    pCertFile = OpcUa_Null;

    if(pCertX509 == OpcUa_Null)
    {
        /* error in OpenSSL - maybe certificate file was corrupt */
        return OpcUa_Bad;
    }

    /* prepare container */
    memset(a_pCertificate, 0, sizeof(OpcUa_ByteString));

    /* get required length for conversion target buffer */
    a_pCertificate->Length = i2d_X509(  pCertX509,
                                            NULL);

    if(a_pCertificate->Length <= 0)
    {
        /* conversion to DER not possible */
        uStatus = OpcUa_Bad;
    }

    /* allocate conversion target buffer */
    a_pCertificate->Data = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(a_pCertificate->Length);
    OpcUa_GotoErrorIfAllocFailed(a_pCertificate->Data);

    /* convert into DER */
    a_pCertificate->Length = i2d_X509(  pCertX509,
                                            &(a_pCertificate->Data));
    if(a_pCertificate->Length <= 0)
    {
        /* conversion to DER not possible */
        uStatus = OpcUa_Bad;
    }
    else
    {
        /* correct pointer incrementation by i2d_X509() */
        a_pCertificate->Data -= a_pCertificate->Length;
    }

    X509_free(pCertX509);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pCertX509 != OpcUa_Null)
    {
        X509_free(pCertX509);
    }

    if(pPKCS12Cert != OpcUa_Null)
    {
        OPENSSL_free(pPKCS12Cert);
    }

    if(a_pCertificate->Data != OpcUa_Null)
    {
        OpcUa_P_Memory_Free(a_pCertificate->Data);
        a_pCertificate->Data = OpcUa_Null;
    }

    if(pCertFile != OpcUa_Null)
    {
        BIO_free(pCertFile);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_X509_GetPublicKey
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_GetPublicKey(
    OpcUa_CryptoProvider*       a_pProvider,
    OpcUa_ByteString*           a_pCertificate,
    OpcUa_StringA               a_password,             /* this could be optional */
    OpcUa_Key*                  a_pPublicKey)
{
    EVP_PKEY*           pPublicKey      = OpcUa_Null;
    RSA*                pRsaPublicKey   = OpcUa_Null;
    X509*               pCertificate    = OpcUa_Null;
    OpcUa_Byte*         pBuffer         = OpcUa_Null;
    BIO*                bi;

    OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "X509_GetPublicKey");

    OpcUa_ReferenceParameter(a_password);

    OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
    OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);
    OpcUa_ReturnErrorIfArgumentNull(a_pCertificate->Data);
    OpcUa_ReturnErrorIfArgumentNull(a_pPublicKey);

    bi = BIO_new(BIO_s_mem());
    BIO_write(bi, a_pCertificate->Data, a_pCertificate->Length);

    pCertificate = d2i_X509_bio(bi, NULL);

    BIO_free(bi);

    if (pCertificate == OpcUa_Null)
    {
        return OpcUa_BadCertificateInvalid;
    }

    /* get EVP_PKEY from X509 certificate */
    pPublicKey = X509_get_pubkey(pCertificate);
    if (pPublicKey == OpcUa_Null)
    {
        X509_free(pCertificate);
        return OpcUa_BadCertificateInvalid;
    }

    /* free X509 certificate, since not needed anymore */
    X509_free(pCertificate);

    switch(EVP_PKEY_type(pPublicKey->type))
    {
    case EVP_PKEY_RSA:

        /* allocate memory for RSA key */

        /* get RSA public key from EVP_PKEY */
        pRsaPublicKey = EVP_PKEY_get1_RSA(pPublicKey);

        /* allocate memory for DER encoded bytestring */
        a_pPublicKey->Key.Length = i2d_RSAPublicKey(pRsaPublicKey, OpcUa_Null);

        if(a_pPublicKey->Key.Data == OpcUa_Null)
        {
            RSA_free(pRsaPublicKey);
            pRsaPublicKey = OpcUa_Null;

            EVP_PKEY_free(pPublicKey);
            pPublicKey = OpcUa_Null;

            OpcUa_ReturnStatusCode;
        }

        /* convert RSA key to DER encoded bytestring */
        pBuffer = a_pPublicKey->Key.Data;
        a_pPublicKey->Key.Length = i2d_RSAPublicKey(pRsaPublicKey, &pBuffer);
        a_pPublicKey->Type = OpcUa_Crypto_KeyType_Rsa_Public;

        /* free memory for RSA key */
        RSA_free(pRsaPublicKey);
        pRsaPublicKey = OpcUa_Null;

        break;

    case EVP_PKEY_EC:
        uStatus =  OpcUa_BadNotSupported;
        OpcUa_GotoErrorIfBad(uStatus);

    case EVP_PKEY_DSA:
        uStatus =  OpcUa_BadNotSupported;
        OpcUa_GotoErrorIfBad(uStatus);

    case EVP_PKEY_DH:
        uStatus =  OpcUa_BadNotSupported;
        OpcUa_GotoErrorIfBad(uStatus);

    default:
        uStatus =  OpcUa_BadNotSupported;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /*** clean up ***/
    EVP_PKEY_free(pPublicKey);
    pPublicKey = OpcUa_Null;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    EVP_PKEY_free(pPublicKey);
    pPublicKey = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_X509_GetPrivateKey
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_GetPrivateKey(
    OpcUa_CryptoProvider*       a_pProvider,
    OpcUa_StringA               a_certificateFileName,
    OpcUa_StringA               a_password,             /* this is optional */
    OpcUa_Key*                  a_pPrivateKey)
{
    BIO*            pCertFile       = OpcUa_Null;
    PKCS12*         pPKCS12Cert     = OpcUa_Null;
    EVP_PKEY*       pPrivateKey     = OpcUa_Null;
    RSA*            pRsaPrivateKey  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "X509_GetPrivateKey");

    OpcUa_ReferenceParameter(a_pProvider);

    OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
    OpcUa_ReturnErrorIfArgumentNull(a_certificateFileName);
    OpcUa_ReturnErrorIfArgumentNull(a_pPrivateKey);

    /* import certificate from file by the given encoding type */
    pCertFile = BIO_new_file((const char*)a_certificateFileName, "r");
    OpcUa_ReturnErrorIfArgumentNull(pCertFile);

    /* convert certificate file handle to PKCS12 structure */
    d2i_PKCS12_bio(pCertFile, &pPKCS12Cert);

    /* close certificat file handle */
    BIO_free(pCertFile);

    if(pPKCS12Cert != OpcUa_Null)
    {
        /* get the private key from the PKCS12 structure*/
        PKCS12_parse(pPKCS12Cert, a_password, &pPrivateKey, OpcUa_Null, OpcUa_Null);
    }
    else
    {
        uStatus = OpcUa_Bad;
        OpcUa_ReturnStatusCode;
    }

    if(pPKCS12Cert != OpcUa_Null)
    {
        PKCS12_free(pPKCS12Cert);
        pPKCS12Cert = OpcUa_Null;
    }

    switch(EVP_PKEY_type(pPrivateKey->type))
    {
    case EVP_PKEY_RSA:
        {
            /* convert to intermediary openssl struct */
            pRsaPrivateKey = EVP_PKEY_get1_RSA(pPrivateKey);
            EVP_PKEY_free(pPrivateKey);
            OpcUa_GotoErrorIfNull(pRsaPrivateKey, OpcUa_Bad);

            /* get required length */
            a_pPrivateKey->Key.Length = i2d_RSAPrivateKey(pRsaPrivateKey, OpcUa_Null);
            OpcUa_GotoErrorIfTrue(a_pPrivateKey->Key.Length <= 0, OpcUa_Bad);

            if(a_pPrivateKey->Key.Data == OpcUa_Null)
            {
                OpcUa_ReturnStatusCode;
            }

            /* do real conversion */
            a_pPrivateKey->Key.Length = i2d_RSAPrivateKey(  pRsaPrivateKey,
                                                            &a_pPrivateKey->Key.Data);
            OpcUa_GotoErrorIfTrue(a_pPrivateKey->Key.Length <= 0, OpcUa_Bad);

            if(pRsaPrivateKey != OpcUa_Null)
            {
                RSA_free(pRsaPrivateKey);
                pRsaPrivateKey = OpcUa_Null;
            }

            /* correct buffer pointer */
            a_pPrivateKey->Key.Data -= a_pPrivateKey->Key.Length;

            a_pPrivateKey->Type = OpcUa_Crypto_KeyType_Rsa_Private;

            break;
        }
    case EVP_PKEY_EC:
    case EVP_PKEY_DSA:
    case EVP_PKEY_DH:
    default:
        {
            uStatus =  OpcUa_BadNotSupported;
            OpcUa_GotoErrorIfBad(uStatus);
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pRsaPrivateKey != OpcUa_Null)
    {
        RSA_free(pRsaPrivateKey);
    }

    if(pPrivateKey != OpcUa_Null)
    {
        EVP_PKEY_free(pPrivateKey);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_X509_GetSignature
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_GetSignature(
    OpcUa_CryptoProvider*       a_pProvider,
    OpcUa_ByteString*           a_pCertificate,
    OpcUa_Signature*            a_pSignature)
{
    X509*                   pX509Certificate    = OpcUa_Null;
    const unsigned char*    pTemp               = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "X509_GetSignature");

    OpcUa_ReferenceParameter(a_pProvider);

    OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
    OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);
    OpcUa_ReturnErrorIfArgumentNull(a_pSignature);

    /* d2i_X509 modifies the given pointer -> use local replacement */
    pTemp = a_pCertificate->Data;

    d2i_X509(&pX509Certificate, &pTemp, a_pCertificate->Length);

    if(pX509Certificate == OpcUa_Null)
    {
        uStatus = OpcUa_Bad;
        OpcUa_GotoError;
    }

    a_pSignature->Signature.Length = pX509Certificate->signature->length;

    if(a_pSignature->Signature.Data == OpcUa_Null)
    {
        X509_free(pX509Certificate);
        OpcUa_ReturnStatusCode;
    }

    OpcUa_P_Memory_MemCpy(  a_pSignature->Signature.Data,
                            pX509Certificate->signature->length,
                            pX509Certificate->signature->data,
                            pX509Certificate->signature->length);

    if(a_pSignature->Signature.Data == OpcUa_Null)
    {
        uStatus = OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    a_pSignature->Algorithm = OBJ_obj2nid(pX509Certificate->sig_alg->algorithm);

    if(a_pSignature->Algorithm == NID_undef)
    {
        uStatus = OpcUa_Bad;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(pX509Certificate != OpcUa_Null)
    {
        X509_free(pX509Certificate);
        pX509Certificate = OpcUa_Null;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pX509Certificate != OpcUa_Null)
    {
        X509_free(pX509Certificate);
        pX509Certificate = OpcUa_Null;
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_X509_GetCertificateThumbprint
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_GetCertificateThumbprint(
    OpcUa_CryptoProvider*       a_pProvider,
    OpcUa_ByteString*           a_pCertificate,
    OpcUa_ByteString*           a_pCertificateThumbprint)
{
    X509*                   pX509Certificate    = OpcUa_Null;
    const unsigned char*    pTemp               = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "X509_GetCertificateThumbprint");

    OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
    OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);
    OpcUa_ReturnErrorIfArgumentNull(a_pCertificate->Data);
    OpcUa_ReturnErrorIfArgumentNull(a_pCertificateThumbprint);

    /* SHA-1 produces 20 bytes */
    a_pCertificateThumbprint->Length = sizeof(pX509Certificate->sha1_hash);

    if(a_pCertificateThumbprint->Data == OpcUa_Null)
    {
        OpcUa_ReturnStatusCode;
    }

    /* d2i_X509 modifies the given pointer -> use local replacement */
    pTemp = a_pCertificate->Data;

    d2i_X509(&pX509Certificate, &pTemp, a_pCertificate->Length);

    if(pX509Certificate == OpcUa_Null)
    {
        uStatus = OpcUa_Bad;
        OpcUa_GotoError;
    }

    /* update pX509Certificate->sha1_hash */
    X509_check_purpose(pX509Certificate, -1, 0);
    uStatus = OpcUa_P_Memory_MemCpy(a_pCertificateThumbprint->Data, a_pCertificateThumbprint->Length,
                                    pX509Certificate->sha1_hash, sizeof(pX509Certificate->sha1_hash));
    X509_free(pX509Certificate);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_REQUIRE_OPENSSL */

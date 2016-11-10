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
#include <opcua_p_memory.h>

#if OPCUA_REQUIRE_OPENSSL

/* System Headers */
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>

/* own headers */
#include <opcua_p_openssl.h>
#include <opcua_p_pki.h>

/*============================================================================
 * OpcUa_P_OpenSSL_RSA_GenerateKeys
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_GenerateKeys(
    OpcUa_CryptoProvider*   a_pProvider,
    OpcUa_UInt32            a_bits,
    OpcUa_Key*              a_pPublicKey,
    OpcUa_Key*              a_pPrivateKey)
{
    RSA*            pRsa        = OpcUa_Null;
    unsigned char*  pData;

    OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "RSA_GenerateKeys");

    OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
    OpcUa_ReturnErrorIfArgumentNull(a_pPublicKey);
    OpcUa_ReturnErrorIfArgumentNull(a_pPrivateKey);

    OpcUa_ReferenceParameter(a_pProvider);

    a_pPublicKey->Key.Data      = OpcUa_Null;
    a_pPrivateKey->Key.Data     = OpcUa_Null;

    if((a_bits < a_pProvider->MinimumAsymmetricKeyLength*8) || (a_bits > a_pProvider->MaximumAsymmetricKeyLength*8))
    {
        uStatus = OpcUa_BadInvalidArgument;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    pRsa = RSA_generate_key(a_bits, RSA_F4, NULL, OpcUa_Null);
    OpcUa_GotoErrorIfNull(pRsa, OpcUa_Bad);

    /* get required length */
    a_pPublicKey->Key.Length = i2d_RSAPublicKey(pRsa, NULL);
    OpcUa_GotoErrorIfTrue((a_pPublicKey->Key.Length <= 0), OpcUa_Bad);

    /* allocate target buffer */
    a_pPublicKey->Key.Data = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(a_pPublicKey->Key.Length);
    OpcUa_GotoErrorIfAllocFailed(a_pPublicKey->Key.Data);

    pData = a_pPublicKey->Key.Data;
    a_pPublicKey->Key.Length = i2d_RSAPublicKey(pRsa, &pData);

    /* get required length */
    a_pPrivateKey->Key.Length = i2d_RSAPrivateKey(pRsa, NULL);
    OpcUa_GotoErrorIfTrue((a_pPrivateKey->Key.Length <= 0), OpcUa_Bad);

    /* allocate target buffer */
    a_pPrivateKey->Key.Data = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(a_pPrivateKey->Key.Length);
    OpcUa_GotoErrorIfAllocFailed(a_pPrivateKey->Key.Data);

    pData = a_pPrivateKey->Key.Data;
    a_pPrivateKey->Key.Length = i2d_RSAPrivateKey(pRsa, &pData);

    /* clean up */
    RSA_free(pRsa);

    a_pPublicKey->Type = OpcUa_Crypto_KeyType_Rsa_Public;
    a_pPrivateKey->Type = OpcUa_Crypto_KeyType_Rsa_Private;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(a_pPublicKey->Key.Data != OpcUa_Null)
    {
        OpcUa_P_Memory_Free(a_pPublicKey->Key.Data);
        a_pPublicKey->Key.Data = OpcUa_Null;
    }

    if(pRsa != OpcUa_Null)
    {
        RSA_free(pRsa);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_RSA_LoadPrivateKeyFromFile
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_LoadPrivateKeyFromFile(
    OpcUa_StringA           a_privateKeyFile,
    OpcUa_P_FileFormat      a_fileFormat,
    OpcUa_StringA           a_password,         /* optional: just needed encrypted PEM */
    OpcUa_ByteString*       a_pPrivateKey)
{
    BIO*            pPrivateKeyFile     = OpcUa_Null;
    RSA*            pRsaPrivateKey      = OpcUa_Null;
    EVP_PKEY*       pEvpKey             = OpcUa_Null;
    unsigned char*  pData;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "RSA_LoadPrivateKeyFromFile");

    /* check parameters */
    OpcUa_ReturnErrorIfArgumentNull(a_privateKeyFile);
    OpcUa_ReturnErrorIfArgumentNull(a_pPrivateKey);

    if(a_fileFormat == OpcUa_Crypto_Encoding_Invalid)
    {
        return OpcUa_BadInvalidArgument;
    }

    OpcUa_ReferenceParameter(a_password);

    /* open file */
    pPrivateKeyFile = BIO_new_file((const char*)a_privateKeyFile, "r");
    OpcUa_ReturnErrorIfArgumentNull(pPrivateKeyFile);

    /* read and convert file */
    switch(a_fileFormat)
    {
    case OpcUa_Crypto_Encoding_PEM:
        {
            /* read from file */
            pEvpKey = PEM_read_bio_PrivateKey(  pPrivateKeyFile,    /* file                 */
                                                NULL,               /* key struct           */
                                                0,                  /* password callback    */
                                                a_password);        /* default passphrase or arbitrary handle */
            OpcUa_GotoErrorIfNull(pEvpKey, OpcUa_Bad);
            pRsaPrivateKey = EVP_PKEY_get1_RSA(pEvpKey);
            OpcUa_GotoErrorIfNull(pRsaPrivateKey, OpcUa_Bad);
            EVP_PKEY_free(pEvpKey);
            pEvpKey = NULL;
            break;
        }
    case OpcUa_Crypto_Encoding_PKCS12:
        {
            int iResult = 0;

            /* read from file. */
            PKCS12* pPkcs12 = d2i_PKCS12_bio(pPrivateKeyFile, NULL);

            if(pPkcs12 == NULL)
            {
                OpcUa_GotoErrorWithStatus(OpcUa_Bad);
            }

            /* parse the certificate. */
            iResult = PKCS12_parse(pPkcs12, a_password, &pEvpKey, NULL, NULL);
            PKCS12_free(pPkcs12);

            if(iResult <= 0)
            {
                OpcUa_GotoErrorWithStatus(OpcUa_Bad);
            }

            /* convert to intermediary openssl struct */
            pRsaPrivateKey = EVP_PKEY_get1_RSA(pEvpKey);
            OpcUa_GotoErrorIfNull(pRsaPrivateKey, OpcUa_Bad);
            EVP_PKEY_free(pEvpKey);
            pEvpKey = NULL;
            break;
        }
    case OpcUa_Crypto_Encoding_DER:
        {
            pRsaPrivateKey = d2i_RSAPrivateKey_bio(pPrivateKeyFile, OpcUa_Null);
            OpcUa_GotoErrorIfNull(pRsaPrivateKey, OpcUa_Bad);
            break;
        }
    default:
        {
            uStatus = OpcUa_BadNotSupported;
            OpcUa_GotoError;
        }
    }

    /* get required length */
    a_pPrivateKey->Length = i2d_RSAPrivateKey(pRsaPrivateKey, OpcUa_Null);
    OpcUa_GotoErrorIfTrue((a_pPrivateKey->Length <= 0), OpcUa_Bad);

    /* allocate target buffer */
    a_pPrivateKey->Data = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(a_pPrivateKey->Length);
    OpcUa_GotoErrorIfAllocFailed(a_pPrivateKey->Data);

    /* do real conversion */
    pData = a_pPrivateKey->Data;
    a_pPrivateKey->Length = i2d_RSAPrivateKey(pRsaPrivateKey, &pData);
    OpcUa_GotoErrorIfTrue((a_pPrivateKey->Length <= 0), OpcUa_Bad);

    RSA_free(pRsaPrivateKey);
    BIO_free(pPrivateKeyFile);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pEvpKey)
    {
        EVP_PKEY_free(pEvpKey);
    }

    if(a_pPrivateKey != OpcUa_Null)
    {
        if(a_pPrivateKey->Data != OpcUa_Null)
        {
            OpcUa_P_Memory_Free(a_pPrivateKey->Data);
            a_pPrivateKey->Data = OpcUa_Null;
            a_pPrivateKey->Length = -1;
        }
    }

    if(pPrivateKeyFile != NULL)
    {
        BIO_free(pPrivateKeyFile);
    }

    if(pRsaPrivateKey != NULL)
    {
        RSA_free(pRsaPrivateKey);
    }

OpcUa_FinishErrorHandling;
}

/*===========================================================================*
OpcUa_P_OpenSSL_RSA_Public_GetKeyLength
*===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Public_GetKeyLength(
    OpcUa_CryptoProvider*   a_pProvider,
    OpcUa_Key               a_publicKey,
    OpcUa_UInt32*           a_pKeyLen)
{
    EVP_PKEY*       pPublicKey      = OpcUa_Null;
    const unsigned char *pData;

    OpcUa_UInt32    uKeySize            = 0;

    OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "RSA_Public_GetKeyLength");

    OpcUa_ReferenceParameter(a_pProvider);

    OpcUa_ReturnErrorIfArgumentNull(a_publicKey.Key.Data);
    OpcUa_ReturnErrorIfArgumentNull(a_pKeyLen);

    *a_pKeyLen = 0;

    if(a_publicKey.Type != OpcUa_Crypto_KeyType_Rsa_Public)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    pData = a_publicKey.Key.Data;
    pPublicKey = d2i_PublicKey(EVP_PKEY_RSA, OpcUa_Null, &pData, a_publicKey.Key.Length);

    if(pPublicKey == OpcUa_Null)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    uKeySize = RSA_size(pPublicKey->pkey.rsa);

    if((uKeySize < a_pProvider->MinimumAsymmetricKeyLength) || (uKeySize > a_pProvider->MaximumAsymmetricKeyLength))
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadSecurityConfig);
    }

    *a_pKeyLen = uKeySize*8;

    EVP_PKEY_free(pPublicKey);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pPublicKey != OpcUa_Null)
    {
        EVP_PKEY_free(pPublicKey);
    }

    *a_pKeyLen = (OpcUa_UInt32)-1;

OpcUa_FinishErrorHandling;
}

/*** RSA ASYMMETRIC ENCRYPTION ***/

/*===========================================================================*
OpcUa_P_OpenSSL_RSA_Public_Encrypt
*===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Public_Encrypt(
    OpcUa_CryptoProvider*   a_pProvider,
    OpcUa_Byte*             a_pPlainText,
    OpcUa_UInt32            a_plainTextLen,
    OpcUa_Key*              a_publicKey,
    OpcUa_Int16             a_padding,
    OpcUa_Byte*             a_pCipherText,
    OpcUa_UInt32*           a_pCipherTextLen)
{
    EVP_PKEY*       pPublicKey      = OpcUa_Null;

    OpcUa_UInt32    uKeySize            = 0;
    OpcUa_UInt32    uEncryptedDataSize  = 0;
    OpcUa_UInt32    uPlainTextPosition  = 0;
    OpcUa_UInt32    uCipherTextPosition = 0;
    OpcUa_UInt32    uBytesToEncrypt     = 0;
    OpcUa_Int32     iEncryptedBytes     = 0;
    const unsigned char *pData;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "RSA_Public_Encrypt");

    OpcUa_ReferenceParameter(a_pProvider);

    OpcUa_ReturnErrorIfArgumentNull(a_publicKey);
    OpcUa_ReturnErrorIfArgumentNull(a_publicKey->Key.Data);
    OpcUa_ReturnErrorIfArgumentNull(a_pCipherTextLen);

    *a_pCipherTextLen = 0;

    if((OpcUa_Int32)a_plainTextLen < 1)
    {
        uStatus = OpcUa_BadInvalidArgument;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(a_publicKey->Type != OpcUa_Crypto_KeyType_Rsa_Public)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    pData = a_publicKey->Key.Data;
    pPublicKey = d2i_PublicKey(EVP_PKEY_RSA, OpcUa_Null, &pData, a_publicKey->Key.Length);

    if(pPublicKey == OpcUa_Null)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    uKeySize = RSA_size(pPublicKey->pkey.rsa);

    /* check padding type */
    switch(a_padding)
    {
    case RSA_PKCS1_PADDING:
        {
            if(uKeySize <= 11)
            {
                OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
            }
            uEncryptedDataSize = uKeySize - 11;
            break;
        }
    case RSA_PKCS1_OAEP_PADDING:
        {
            if(uKeySize <= 42)
            {
                OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
            }
            uEncryptedDataSize = uKeySize - 42;
            break;
        }
    case RSA_NO_PADDING:
        {
            if(uKeySize == 0)
            {
                OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
            }
            uEncryptedDataSize = uKeySize;
            break;
        }
    default:
        {
            OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
        }
    }

    if(a_plainTextLen < uEncryptedDataSize)
    {
        uBytesToEncrypt = a_plainTextLen;
    }
    else
    {
        uBytesToEncrypt = uEncryptedDataSize;
    }

    while(uPlainTextPosition < a_plainTextLen)
    {

        /* the last part could be smaller */
        if((a_plainTextLen >= uEncryptedDataSize) && ((a_plainTextLen - uPlainTextPosition) < uEncryptedDataSize))
        {
            uBytesToEncrypt = a_plainTextLen - uPlainTextPosition;
        }

        if((a_pCipherText != OpcUa_Null) && (a_pPlainText != OpcUa_Null))
        {
            iEncryptedBytes = RSA_public_encrypt(   uBytesToEncrypt,      /* how much to encrypt  */
                                                    a_pPlainText + uPlainTextPosition,      /* what to encrypt      */
                                                    a_pCipherText + uCipherTextPosition,/* where to encrypt     */
                                                    pPublicKey->pkey.rsa,                  /* public key           */
                                                    a_padding);        /* padding mode         */
            if(iEncryptedBytes < 0)
            {
                uStatus = OpcUa_Bad;
                OpcUa_GotoError;
            }

        }
        else
        {
            iEncryptedBytes = uKeySize;
        }

        *a_pCipherTextLen = *a_pCipherTextLen + iEncryptedBytes;
        uCipherTextPosition += uKeySize;
        uPlainTextPosition  += uBytesToEncrypt;
    }

    EVP_PKEY_free(pPublicKey);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pPublicKey != OpcUa_Null)
    {
        EVP_PKEY_free(pPublicKey);
    }

    *a_pCipherTextLen = (OpcUa_UInt32)-1;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_RSA_Private_Decrypt
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Private_Decrypt(
    OpcUa_CryptoProvider*   a_pProvider,
    OpcUa_Byte*             a_pCipherText,
    OpcUa_UInt32            a_cipherTextLen,
    OpcUa_Key*              a_privateKey,
    OpcUa_Int16             a_padding,
    OpcUa_Byte*             a_pPlainText,
    OpcUa_UInt32*           a_pPlainTextLen)
{
    EVP_PKEY*       pPrivateKey     = OpcUa_Null;

    OpcUa_UInt32    keySize         = 0;
    OpcUa_Int32     decryptedBytes  = 0;
    OpcUa_UInt32    iCipherText     = 0;
    /* OpcUa_UInt32 iPlainTextLen   = 0; */
    OpcUa_UInt32    decDataSize     = 0;

    const unsigned char *pData;

    OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "RSA_Private_Decrypt");

    OpcUa_ReferenceParameter(a_pProvider);

    OpcUa_ReturnErrorIfArgumentNull(a_pCipherText);
    OpcUa_ReturnErrorIfArgumentNull(a_privateKey);
    OpcUa_ReturnErrorIfArgumentNull(a_privateKey->Key.Data);
    OpcUa_ReturnErrorIfArgumentNull(a_pPlainTextLen);

    *a_pPlainTextLen = 0;

    if(a_privateKey->Type != OpcUa_Crypto_KeyType_Rsa_Private)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    pData = a_privateKey->Key.Data;
    pPrivateKey = d2i_PrivateKey(EVP_PKEY_RSA,OpcUa_Null, &pData, a_privateKey->Key.Length);

    if (pPrivateKey == OpcUa_Null)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    keySize = RSA_size(pPrivateKey->pkey.rsa);

    if(keySize == 0)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    if((a_cipherTextLen%keySize) != 0)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    /* check padding type */
    switch(a_padding)
    {
    case RSA_PKCS1_PADDING:
        {
            if(keySize <= 11)
            {
                OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
            }
            decDataSize = keySize - 11;
            break;
        }
    case RSA_PKCS1_OAEP_PADDING:
        {
            if(keySize <= 42)
            {
                OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
            }
            decDataSize = keySize - 42;
            break;
        }
    case RSA_NO_PADDING:
        {
            decDataSize = keySize;
            break;
        }
    default:
        {
            OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
        }
    }

    while(iCipherText < a_cipherTextLen)
    {
        if(a_pPlainText != OpcUa_Null)
        {
            decryptedBytes = RSA_private_decrypt(   keySize,                            /* how much to decrypt  */
                                                    a_pCipherText + iCipherText,        /* what to decrypt      */
                                                    a_pPlainText + (*a_pPlainTextLen),  /* where to decrypt     */
                                                    pPrivateKey->pkey.rsa,              /* private key          */
                                                    a_padding);                         /* padding mode         */

            /* if decryption fails return the same result as if signature check fails */
            if(decryptedBytes < 0)
            {
                /* continue decrypting the message in constant time */
                uStatus = OpcUa_BadSignatureInvalid;
                decryptedBytes = decDataSize;
                /* do not leak timing information by skipping the memcpy */
                memmove(a_pPlainText + (*a_pPlainTextLen), a_pCipherText + iCipherText, decryptedBytes);
            }

        }
        else
        {
            decryptedBytes = decDataSize;
        }

        *a_pPlainTextLen = *a_pPlainTextLen + decryptedBytes;
        iCipherText = iCipherText + keySize;
    }


    EVP_PKEY_free(pPrivateKey);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pPrivateKey != OpcUa_Null)
    {
        EVP_PKEY_free(pPrivateKey);
    }

    *a_pPlainTextLen = (OpcUa_UInt32)-1;

OpcUa_FinishErrorHandling;
}

/*** RSA ASYMMETRIC SIGNATURE ***/

/*===========================================================================*
OpcUa_P_OpenSSL_RSA_Private_Sign
 *===========================================================================*/
/*
 * ToDo: problems with RSA_PKCS1_OAEP_PADDING -> RSA_PKCS1_PSS_PADDING is
 * needed (Version 0.9.9); RSA_PKCS1_OAEP_PADDING is just for encryption
 */
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Private_Sign(
    OpcUa_CryptoProvider* a_pProvider,
    OpcUa_ByteString      a_data,
    OpcUa_Key*            a_privateKey,
    OpcUa_Int16           a_padding,          /* e.g. RSA_PKCS1_PADDING */
    OpcUa_ByteString*     a_pSignature)       /* output length >= key length */
{
    EVP_PKEY*               pSSLPrivateKey  = OpcUa_Null;
    const unsigned char*    pData           = OpcUa_Null;
    int                     iErr            = 0;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "RSA_Private_Sign");

    /* unused parameters */
    OpcUa_ReferenceParameter(a_pProvider);
    OpcUa_ReferenceParameter(a_padding);

    /* check parameters */
    OpcUa_ReturnErrorIfArgumentNull(a_data.Data);
    OpcUa_ReturnErrorIfArgumentNull(a_privateKey);
    OpcUa_ReturnErrorIfArgumentNull(a_pSignature);
    OpcUa_ReturnErrorIfArgumentNull(a_pSignature->Data);
    pData = a_privateKey->Key.Data;
    OpcUa_ReturnErrorIfArgumentNull(pData);
    OpcUa_ReturnErrorIfTrue((a_privateKey->Type != OpcUa_Crypto_KeyType_Rsa_Private), OpcUa_BadInvalidArgument);

    /* convert private key and check key length against buffer length */
    pSSLPrivateKey = d2i_PrivateKey(EVP_PKEY_RSA, OpcUa_Null, &pData, a_privateKey->Key.Length);
    OpcUa_GotoErrorIfTrue((pSSLPrivateKey == OpcUa_Null), OpcUa_BadUnexpectedError);
    OpcUa_GotoErrorIfTrue((a_pSignature->Length < RSA_size(pSSLPrivateKey->pkey.rsa)), OpcUa_BadInvalidArgument);

    /* sign data */
    iErr = RSA_sign(a_data.Length == 32 ? NID_sha256 : NID_sha1, a_data.Data, a_data.Length, a_pSignature->Data, (unsigned int*)&a_pSignature->Length, pSSLPrivateKey->pkey.rsa);
    OpcUa_GotoErrorIfTrue((iErr != 1), OpcUa_BadUnexpectedError);

    /* free internal key representation */
    EVP_PKEY_free(pSSLPrivateKey);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pSSLPrivateKey != OpcUa_Null)
    {
        EVP_PKEY_free(pSSLPrivateKey);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_RSA_Public_Verify
 *===========================================================================*/
/*
ToDo: problems with RSA_PKCS1_OAEP_PADDING -> find solution
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Public_Verify(
    OpcUa_CryptoProvider* a_pProvider,
    OpcUa_ByteString      a_data,
    OpcUa_Key*            a_publicKey,
    OpcUa_Int16           a_padding,
    OpcUa_ByteString*     a_pSignature)
{
    EVP_PKEY*       pPublicKey      = OpcUa_Null;
    OpcUa_Int32     keySize         = 0;
    const unsigned char *pData;

    OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "RSA_Public_Verify");

    OpcUa_ReferenceParameter(a_pProvider);
    OpcUa_ReferenceParameter(a_padding);

    OpcUa_ReturnErrorIfArgumentNull(a_data.Data);
    OpcUa_ReturnErrorIfArgumentNull(a_publicKey);
    OpcUa_ReturnErrorIfArgumentNull(a_publicKey->Key.Data);
    OpcUa_ReturnErrorIfArgumentNull(a_pSignature);

    if(a_publicKey->Type != OpcUa_Crypto_KeyType_Rsa_Public)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    pData = a_publicKey->Key.Data;
    pPublicKey = d2i_PublicKey(EVP_PKEY_RSA,OpcUa_Null, &pData, a_publicKey->Key.Length);

    if(pPublicKey == OpcUa_Null)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    keySize = RSA_size(pPublicKey->pkey.rsa);

    if(keySize == 0)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }

    if((a_pSignature->Length%keySize) != 0)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadSignatureInvalid);
    }

    if(RSA_verify(a_data.Length == 32 ? NID_sha256 : NID_sha1, a_data.Data, a_data.Length, a_pSignature->Data, a_pSignature->Length, pPublicKey->pkey.rsa) != 1)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadSignatureInvalid);
    }

    EVP_PKEY_free(pPublicKey);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if (pPublicKey != OpcUa_Null)
    {
        EVP_PKEY_free(pPublicKey);
    }

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_REQUIRE_OPENSSL */

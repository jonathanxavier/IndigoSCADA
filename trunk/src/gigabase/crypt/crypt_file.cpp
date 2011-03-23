//-< CRYPT_FILE.CPP >------------------------------------------------*--------*
// GigaBASE                  Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
//-------------------------------------------------------------------*--------*

#define INSIDE_GIGABASE

#ifdef __BORLANDC__
# pragma warn -8022
#endif
#include "gigabase.h"
#include "crypt_file.h"
#include "rc4/arc4.h"
#include "md5/md5.h"


BEGIN_GIGABASE_NAMESPACE

dbCryptFile::dbCryptFile(const char* cryptKey)
{
    key = cryptKey;
}

int dbCryptFile::open(char_t const* fileName, int attr)
{
    int ret = dbOSFile::open(fileName,attr);
    if (ret!=ok) { 
        return ret;    
    }
    
    unsigned char buf[dbPageSize];
    ret = dbOSFile::read(0, buf, dbPageSize);
    if (ret != ok && ret != eof) { 
        return ret;
    }

    Crc* crc = (Crc*)(buf+sizeof(buf)-sizeof(Crc));
    MD5_CTX delta;
    MD5_CTX tick;
    
    if (ret==ok) {
        MD5Init(&tick);
        MD5Update(&tick, (unsigned char*)key.c_str(), key.size());
        MD5Update(&tick, (unsigned char*)&crc->tick, sizeof(crc->tick));
        MD5Final(&tick);
        
        memcpy(tick_digest, tick.digest, sizeof(tick_digest));
        CryptoPP::ARC4 write_rc4(tick_digest, sizeof(tick_digest));
        write_rc4.ProcessString(buf, dbPageSize-sizeof(Crc)+(int)(&((Crc*)0l)->tick) );
        
        MD5Init(&delta);
        MD5Update(&delta, (unsigned char*)key.c_str(), key.size());
        MD5Update(&delta, (unsigned char*)&crc->delta, sizeof(crc->delta));
        MD5Final(&delta);
        if (memcmp(crc->digest, delta.digest, sizeof(crc->digest))) { 
            return crc_failed;
        }
        
        open_crc = *crc;
        return ok;
    }

    if (attr&read_only) { 
        return crc_failed;
    }
    
    crc->last_crypted=0;
    crc->delta = generate_delta();
    crc->tick = generate_tick();
    
    MD5Init(&delta);
    MD5Update(&delta, (unsigned char*)key.c_str(), key.size());
    MD5Update(&delta, (unsigned char*)&crc->delta, sizeof(crc->delta));
    MD5Final(&delta);
    memcpy(crc->digest, delta.digest, sizeof(crc->digest));
    
    MD5Init(&tick);
    MD5Update(&tick, (unsigned char*)key.c_str(), key.size());
    MD5Update(&tick, (unsigned char*)&crc->tick, sizeof(crc->tick) );
    MD5Final(&tick);
    
    open_crc=*crc;
    memcpy(tick_digest, tick.digest, sizeof(tick_digest));
    CryptoPP::ARC4 write_rc4(tick_digest, sizeof(tick_digest));
    write_rc4.ProcessString(buf, dbPageSize-sizeof(Crc)+(int)(&((Crc*)0l)->tick)  );
    
    return dbOSFile::write(0, buf, dbPageSize);
}

int dbCryptFile::write_header()
{
    unsigned char buf[dbPageSize];
    Crc* crc=(Crc*)(buf+sizeof(buf)-sizeof(Crc));
    *crc=open_crc;
    
    CryptoPP::ARC4 write_rc4(tick_digest, sizeof(tick_digest));
    write_rc4.ProcessString(buf, dbPageSize-sizeof(Crc)+(int)(&((Crc*)0l)->tick) );
    return dbOSFile::write(0, buf, dbPageSize);
}


int dbCryptFile::write(offs_t pos, void const* ptr, size_t size)
{
    unsigned char buf[dbPageSize];
    
    offs_t from = pos/dbPageSize;
    offs_t to = (pos+size)/dbPageSize;
    if (pos+size > to*dbPageSize) { 
        to += 1;
    }
    
    for (offs_t i=from; i < to; i++)
    {
        int ret=crypt_write(buf, i, pos, ptr, size);
        if (ret!=ok) return ret;
    }

    to -= 1;

    if (to>open_crc.last_crypted)
    {
        for (offs_t i=open_crc.last_crypted+1; i < from; i++)
        {
            memset(buf, 0, dbPageSize);
            crypt_write(buf, i);
        }
        open_crc.last_crypted=to;
        return write_header();
    }
    return ok;
}

int dbCryptFile::crypt_write(unsigned char* buf, offs_t block)
{
    CryptoPP::ARC4 write_rc4(tick_digest, sizeof(tick_digest));
    write_rc4.ProcessString(buf, dbPageSize);
    return dbOSFile::write((block+1)*dbPageSize, buf, dbPageSize);
}

int dbCryptFile::crypt_write(unsigned char* buf, offs_t block, offs_t gpos, void const* gptr, size_t gsize)
{
    offs_t apos=block*dbPageSize;
    
    offs_t pos;
    const void* ptr;
    size_t size;
    
    if (apos>gpos)
    {
        pos = 0;
        ptr = (unsigned char*)gptr+(size_t)(apos-gpos);
        size = gpos+gsize-apos;
        if (size > dbPageSize) { 
            size=dbPageSize;
        }
    }
    else
    {
        pos = gpos-apos;
        ptr = gptr;
        if (pos+gsize > dbPageSize) { 
            size = dbPageSize-pos;
        } else { 
            size = gsize;
        }
    }
    
    int ret;

   if (size != dbPageSize
        && block <= open_crc.last_crypted
        && (ret = crypt_read(buf, block)) != ok) 
    {
        return ret;
    }
    memcpy(buf+pos, ptr, size);
    return crypt_write(buf, block);
}

int dbCryptFile::crypt_read(unsigned char* buf, offs_t block)
{
    int ret = dbOSFile::read((block+1)*dbPageSize, buf, dbPageSize);
    if (ret == ok && block <= open_crc.last_crypted)
    {
        CryptoPP::ARC4 write_rc4(tick_digest, sizeof(tick_digest));
        write_rc4.ProcessString(buf, dbPageSize);
    }
    return ret;
}

int dbCryptFile::read(offs_t pos, void* ptr, size_t size)
{
    offs_t from = pos/dbPageSize;
    offs_t to = (pos+size)/dbPageSize;
    if (pos+size>to*dbPageSize)to++;
    
    for (offs_t i = from; i<to; i++)
    {
        int ret = crypt_read(i, pos, ptr, size);
        if (ret!=ok) { 
            return ret;
        }
    }
    
    return ok;
}

int dbCryptFile::crypt_read(offs_t block, offs_t gpos, void* gptr, size_t gsize)
{
    offs_t apos=block*dbPageSize;
    
    offs_t pos;
    void* ptr;
    size_t size;
    
    if (apos>gpos)
    {
        pos=0;
        ptr = (unsigned char*)gptr+(size_t)(apos-gpos);
        size = gpos+gsize-apos;
        if (size>dbPageSize) { 
            size=dbPageSize;
        }
    } else {
        pos = gpos-apos;
        ptr = gptr;
        if (pos+gsize > dbPageSize) { 
            size = dbPageSize-pos;
        } else { 
            size = gsize;
        }
    }
    
    if (pos == 0 && size == dbPageSize) { 
        return crypt_read((unsigned char*)ptr, block);
    }
    unsigned char buf[dbPageSize];
    
    int ret = crypt_read(buf, block);
    if (ret!=ok) { 
        return ret;
    }
    memcpy(ptr, buf+pos, size);
    return ok;
}

int dbCryptFile::setSize(offs_t offs)
{
    return dbOSFile::setSize(offs+dbPageSize);
}

char_t* dbCryptFile::errorText(int code, char_t* buf, size_t bufSize)
{
    if (code==crc_failed)
    {
        STRNCPY(buf, STRLITERAL("CRC check failed"), bufSize);
        return buf;
    }
    return dbOSFile::errorText(code, buf, bufSize);
}

#ifdef _WIN32
#include <windows.h>

double dbCryptFile::generate_delta()
{
  GUID g;
  CoCreateGuid(&g);
  return g.Data1*g.Data2*g.Data3;
}

int dbCryptFile::generate_tick()
{
  return GetTickCount();
}

#else

double dbCryptFile::generate_delta()
{
   return (double)time(NULL); 
}

int dbCryptFile::generate_tick()
{
    return (int)time(NULL);
}


#endif

END_GIGABASE_NAMESPACE


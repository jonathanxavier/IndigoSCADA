//-< CRYPT_FILE.CPP >------------------------------------------------*--------*
// GigaBASE                  Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
//-------------------------------------------------------------------*--------*

#ifndef __CRYPT_FILE_H__
#define __CRYPT_FILE_H__

#include <string>

BEGIN_GIGABASE_NAMESPACE

/**
 * Crypt file
 */
class GIGABASE_DLL_ENTRY dbCryptFile : public dbOSFile {
 private:
    std::string key;

    int crypt_read(unsigned char* buf,offs_t block);
    int crypt_write(unsigned char* buf,offs_t block);
    
    int crypt_write(unsigned char* buf,offs_t block,offs_t gpos, void const* gptr, size_t gsize);
    int crypt_read(offs_t block,offs_t gpos, void * gptr, size_t gsize);
    
    static double generate_delta();
    static int generate_tick();
    
    struct Crc
    {
        offs_t last_crypted;
        unsigned char digest[16];
        double delta;
        int tick;
    };

    enum CrcStatuc { 
        crc_failed = -2
    };
      

    unsigned char tick_digest[16];
    
    Crc open_crc;
    
    int write_header();
    
  public:
    
    int open(char_t const* fileName, int attr);
    
    virtual int write(offs_t pos, void const* ptr, size_t size);
    virtual int read(offs_t pos, void* ptr, size_t size);
    
    virtual int setSize(offs_t offs);
    
    virtual char_t* errorText(int code, char_t* buf, size_t bufSize);
    
    dbCryptFile(const char* key);
};


END_GIGABASE_NAMESPACE

#endif

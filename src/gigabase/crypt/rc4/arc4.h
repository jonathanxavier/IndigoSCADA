#ifndef CRYPTOPP_ARC4_H
#define CRYPTOPP_ARC4_H

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

//! <a href="http://www.weidai.com/scan-mirror/cs.html#RC4">Alleged RC4</a>
class ARC4 : public RandomNumberGenerator, public StreamCipher, public VariableKeyLength<16, 1, 256>
{
public:
    ARC4(const byte *userKey, unsigned int keyLength=DEFAULT_KEYLENGTH);
    ~ARC4();

    byte GenerateByte();

    byte ProcessByte(byte input);
    void ProcessString(byte *outString, const byte *inString, unsigned int length);
    void ProcessString(byte *inoutString, unsigned int length);

private:
    SecByteBlock m_state;
    byte m_x, m_y;
};

//! Modified ARC4: it discards the first 256 bytes of keystream which may be weaker than the rest
class MARC4 : public ARC4
{
public:
    MARC4(const byte *userKey, unsigned int keyLength=DEFAULT_KEYLENGTH, unsigned int discardBytes=256);
};

NAMESPACE_END

#endif

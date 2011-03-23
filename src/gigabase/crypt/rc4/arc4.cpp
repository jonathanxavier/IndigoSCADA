// arc4.cpp - written and placed in the public domain by Wei Dai

// The ARC4 algorithm was first revealed in an anonymous email to the
// cypherpunks mailing list. This file originally contained some
// code copied from this email. The code has since been rewritten in order
// to clarify the copyright status of this file. It should now be
// completely in the public domain.

#include "pch.h"
#include "arc4.h"

NAMESPACE_BEGIN(CryptoPP)

ARC4::ARC4(const byte *key, unsigned int keyLen)
	: m_state(256), m_x(0), m_y(0)
{
	unsigned int i;
	for (i=0; i<256; i++)
		m_state[i] = i;

	unsigned int keyIndex = 0, stateIndex = 0;
	for (i=0; i<256; i++)
	{
		unsigned int a = m_state[i];
		stateIndex += key[keyIndex] + a;
		stateIndex &= 0xff;
		m_state[i] = m_state[stateIndex];
		m_state[stateIndex] = a;
		if (++keyIndex >= keyLen)
			keyIndex = 0;
	}
}

ARC4::~ARC4()
{
	m_x=0;
	m_y=0;
}

byte ARC4::GenerateByte()
{
	m_x = (m_x+1) & 0xff;
	unsigned int a = m_state[m_x];
	m_y = (m_y+a) & 0xff;
	unsigned int b = m_state[m_y];
	m_state[m_x] = b;
	m_state[m_y] = a;
	return m_state[(a+b) & 0xff];
}

byte ARC4::ProcessByte(byte input)
{
	return input ^ ARC4::GenerateByte();
}

void ARC4::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
	byte *const s=m_state;
	unsigned int x = m_x;
	unsigned int y = m_y;

	while(length--)
	{
		x = (x+1) & 0xff;
		unsigned int a = s[x];
		y = (y+a) & 0xff;
		unsigned int b = s[y];
		s[x] = b;
		s[y] = a;
		*outString++ = *inString++ ^ s[(a+b) & 0xff];
	}

	m_x = x;
	m_y = y;
}

void ARC4::ProcessString(byte *inoutString, unsigned int length)
{
	byte *const s=m_state;
	unsigned int x = m_x;
	unsigned int y = m_y;

	while(length--)
	{
		x = (x+1) & 0xff;
		unsigned int a = s[x];
		y = (y+a) & 0xff;
		unsigned int b = s[y];
		s[x] = b;
		s[y] = a;
		*inoutString++ ^= s[(a+b) & 0xff];
	}

	m_x = x;
	m_y = y;
}

MARC4::MARC4(const byte *userKey, unsigned int keyLength, unsigned int discardBytes)
	: ARC4(userKey, keyLength)
{
	while (discardBytes--)
		MARC4::GenerateByte();
}

NAMESPACE_END

#ifndef CRC_EIGHT_H
#define CRC_EIGHT_H 1

#ifdef __cplusplus
extern "C" {
#endif

unsigned char Slow_CRC_Cal8Bits(unsigned char crc, int Size, unsigned char *Buffer);
unsigned char Quick_CRC_Cal8Bits(unsigned char crc, int Size, unsigned char *Buffer);
unsigned char Fast_CRC_Cal8Bits(unsigned char crc, int Size, unsigned char *Buffer);

#ifdef __cplusplus
}
#endif

#endif

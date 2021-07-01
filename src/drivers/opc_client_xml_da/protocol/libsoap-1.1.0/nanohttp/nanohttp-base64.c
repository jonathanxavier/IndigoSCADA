/******************************************************************
*  $Id: nanohttp-base64.c,v 1.2 2006/02/25 10:09:29 snowdrop Exp $
*
* CSOAP Project:  A http client/server library in C
* Copyright (C) 2003  Ferhat Ayaz
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA  02111-1307, USA.
*
* Email: hero@persua.de
******************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "nanohttp-base64.h"

static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char cd64[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/**
 * encode 3 8-bit binary bytes as 4 '6-bit' characters
 */
static void encodeblock(unsigned char in[3], unsigned char out[4], int len)
{

  out[0] = cb64[in[0] >> 2];
  out[1] = cb64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
  out[2] = (unsigned char)(len > 1 ? cb64[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)] : '=');
  out[3] = (unsigned char)(len > 2 ? cb64[in[2] & 0x3f] : '=');

  return;
}

/**
 * base64 encode a string.
 */
void base64_encode(const unsigned char *instr, unsigned char *outstr)
{
  unsigned char in[3], out[4];
  int i, len;

  while (*instr)
  {
    len = 0;
    for (i = 0; i < 3; i++)
    {
      if ((in[i] = (unsigned char)*instr))
      {
        len++;
        instr++;
      }
    }
    if (len)
    {
      encodeblock(in, out, len);
      for (i = 0; i < 4; i++)
        *outstr++ = out[i];
    }
  }
}

/**
 * decode 4 '6-bit' characters into 3 8-bit binary bytes
 */
static void decodeblock(unsigned char in[4], unsigned char out[3])
{
  out[0] = (unsigned char)(in[0] << 2 | in[1] >> 4);
  out[1] = (unsigned char)(in[1] << 4 | in[2] >> 2);
  out[2] = (unsigned char)(((in[2] << 6) & 0xc0) | in[3]);

  return;
}

/** 
 * decode a base64 encoded string (maybe broken...)
 */
void base64_decode(const unsigned char *instr, unsigned char *outstr)
{
  unsigned char in[4], out[3], v;
  int i, len;

  while (*instr)
  {
    for (len = 0, i = 0; i < 4 && *instr; i++)
    {
      v = 0;
      while (*instr && v == 0)
      {
        v = *instr++;
        v = (unsigned char)((v < 43 || v > 122) ? 0 : cd64[v - 43]);
        if (v)
          v = (unsigned char)((v == '$') ? 0 : v - 61);
      }
      if (*instr)
      {
        len++;
        if (v)
          in[i] = (unsigned char)(v - 1);
      }
      else
      {
        in[i] = 0;
      }
    }
    if (len)
    {
      decodeblock(in, out);
      for (i = 0; i < len - 1; i++)
        *outstr++ = out[i];
    }
  }
}

#ifdef BASE64_TEST_CASE_FROM_RFC2617
#include <stdio.h>
int main(int argc, char **argv) {

  unsigned char *instr = "QWxhZGRpbjpvcGVuIHNlc2FtZQ==";
  unsigned char *result = "Aladdin:open sesame";
  unsigned char instr2[80];
  unsigned char outstr[80];

  bzero(outstr, 80);
  base64_decode(instr, outstr);

  printf("\"%s\" => \"%s\"\n", instr, outstr);
  if (strcmp(outstr, result))
    printf("base64_decode failed\n");

  strcpy(instr2, outstr);

  bzero(outstr, 80);
  base64_encode(instr2, outstr);

  printf("\"%s\" => \"%s\"\n", instr2, outstr);
  if (strcmp(outstr, instr))
    printf("base64_encode failed\n");

  return 0;
}
#endif

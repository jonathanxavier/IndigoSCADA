/* Copyright 2009
 * Kaz Kylheku <kkylheku@gmail.com>
 * Vancouver, Canada
 * All rights reserved.
 *
 * BSD License:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *   3. The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef SFX_H
#define SFX_H

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    sfx_none, sfx_potential, sfx_certain
} sfx_rating_t;

int sfx_determine(const char *, sfx_rating_t *);
int sfx_declare(const char *, sfx_rating_t);
void sfx_check(const char *, const char *, unsigned long);

#ifdef __cplusplus
}
#endif

#define SFX_CHECK(E) (sfx_check(#E, __FILE__, __LINE__), (E))
#define SFX_STRING(E) #E

#endif

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

#ifndef LIST_H
#define LIST_H

#include <limits.h>

#ifdef KAZLIB_SIDEEFFECT_DEBUG
#include "sfx.h"
#define LIST_SFX_CHECK(E) SFX_CHECK(E)
#else
#define LIST_SFX_CHECK(E) (E)
#endif

/*
 * Blurb for inclusion into C++ translation units
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long listcount_t;
#define LISTCOUNT_T_MAX ULONG_MAX

typedef struct lnode_t {
    #if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
    struct lnode_t *list_next;
    struct lnode_t *list_prev;
    void *list_data;
    #else
    int list_dummy;
    #endif
} lnode_t;

typedef struct lnodepool_t {
    #if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
    struct lnode_t *list_pool;
    struct lnode_t *list_free;
    listcount_t list_size;
    #else
    int list_dummy;
    #endif
} lnodepool_t;

typedef struct list_t {
    #if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
    lnode_t list_nilnode;
    listcount_t list_nodecount;
    listcount_t list_maxcount;
    #else
    int list_dummy;
    #endif
} list_t;

lnode_t *lnode_create(void *);
lnode_t *lnode_init(lnode_t *, void *);
void lnode_destroy(lnode_t *);
void lnode_put(lnode_t *, void *);
void *lnode_get(lnode_t *);
int lnode_is_in_a_list(lnode_t *);

#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define lnode_put(N, D)         ((N)->list_data = (D))
#define lnode_get(N)            ((N)->list_data)
#endif

lnodepool_t *lnode_pool_init(lnodepool_t *, lnode_t *, listcount_t);
lnodepool_t *lnode_pool_create(listcount_t);
void lnode_pool_destroy(lnodepool_t *);
lnode_t *lnode_borrow(lnodepool_t *, void *);
void lnode_return(lnodepool_t *, lnode_t *);
int lnode_pool_isempty(lnodepool_t *);
int lnode_pool_isfrom(lnodepool_t *, lnode_t *);

list_t *list_init(list_t *, listcount_t);
list_t *list_create(listcount_t);
void list_destroy(list_t *);
void list_destroy_nodes(list_t *);
void list_return_nodes(list_t *, lnodepool_t *);

listcount_t list_count(list_t *);
int list_isempty(list_t *);
int list_isfull(list_t *);
int list_contains(list_t *, lnode_t *);

void list_append(list_t *, lnode_t *);
void list_prepend(list_t *, lnode_t *);
void list_ins_before(list_t *, lnode_t *, lnode_t *);
void list_ins_after(list_t *, lnode_t *, lnode_t *);

lnode_t *list_first(list_t *);
lnode_t *list_last(list_t *);
lnode_t *list_next(list_t *, lnode_t *);
lnode_t *list_prev(list_t *, lnode_t *);

lnode_t *list_del_first(list_t *);
lnode_t *list_del_last(list_t *);
lnode_t *list_delete(list_t *, lnode_t *);

void list_process(list_t *, void *, void (*)(list_t *, lnode_t *, void *));

int list_verify(list_t *);

#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define lnode_pool_isempty(P)   ((P)->list_free == 0)
#define list_count(L)           ((L)->list_nodecount)
#define list_isempty(L)         ((L)->list_nodecount == 0)
#define list_isfull(L)          (LIST_SFX_CHECK(L)->list_nodecount == (L)->list_maxcount)
#define list_next(L, N)         (LIST_SFX_CHECK(N)->list_next == &(L)->list_nilnode ? NULL : (N)->list_next)
#define list_prev(L, N)         (LIST_SFX_CHECK(N)->list_prev == &(L)->list_nilnode ? NULL : (N)->list_prev)
#define list_first(L)           list_next(LIST_SFX_CHECK(L), &(L)->list_nilnode)
#define list_last(L)            list_prev(LIST_SFX_CHECK(L), &(L)->list_nilnode)
#endif

#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define list_append(L, N)       list_ins_before(LIST_SFX_CHECK(L), N, &(L)->list_nilnode)
#define list_prepend(L, N)      list_ins_after(LIST_SFX_CHECK(L), N, &(L)->list_nilnode)
#define list_del_first(L)       list_delete(LIST_SFX_CHECK(L), list_first(L))
#define list_del_last(L)        list_delete(LIST_SFX_CHECK(L), list_last(L))
#endif

/* destination list on the left, source on the right */

void list_extract(list_t *, list_t *, lnode_t *, lnode_t *);
void list_transfer(list_t *, list_t *, lnode_t *first);
void list_merge(list_t *, list_t *, int (const void *, const void *));
void list_sort(list_t *, int (const void *, const void *));
lnode_t *list_find(list_t *, const void *, int (const void *, const void *));
int list_is_sorted(list_t *, int (const void *, const void *));

#ifdef __cplusplus
}
#endif

#endif

/*
 * Copyright (c) 2000, 2001 Peter Bozarov.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Peter Bozarov.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * A stack implementation. Boring, but useful.
 * Last changed: Dec 07, 2001
 */

# include "local.h"

typedef struct StackElement
{
    void *			seData;
    struct StackElement *	seNext;
} StackElement;
struct Stack
{
    int			stTotal;
    StackElement *	stTop;
};

# define ST_TOP(stack)	 ((Stack*)stack)->stTop
# define ST_TOTAL(stack) ((Stack*)stack)->stTotal

STACK
stkMake(void)
{
    Stack *	stack;

    STDMALLOC(stack,sizeof(Stack),NULL);

    stack->stTop = NULL;
    stack->stTotal = 0;

    return (STACK)stack;
}
void
stkCloseWithFunction(STACK stack,void (*fun)(void*))
{
    StackElement *	stelem;

    stelem = ST_TOP(stack);
    while (stelem)
    {
	StackElement *	tmp;

	tmp = stelem;
	stelem 	= stelem->seNext;

	if (fun)
	    fun(tmp->seData);
	STDFREE(tmp);
    }

    /* Free the stack pointer itself */
    STDFREE( (void*) stack);

    return;
}
void
stkClose(STACK stack)
{
    stkCloseWithFunction(stack,NULL);
}
int
stkPush(STACK stack,void *elem)
{
    StackElement *	stelem;

    STDMALLOC(stelem,sizeof(StackElement),-1);

    stelem->seData	= elem;
    stelem->seNext	= ST_TOP(stack);
    ST_TOP(stack)	= stelem;
    ++ST_TOTAL(stack);

    return 0;
}
void*
stkPop(STACK stack)
{
    StackElement *	stelem;
    void *		data;

    /* Pop the top element from the stack, decrease the stack element
     * count and return the data stored with the top element of the
     * stack.
     */
    stelem = ST_TOP(stack);
    if (!stelem)
	return NULL;

    ST_TOP(stack) = stelem->seNext;
    --ST_TOTAL(stack);

    data = stelem->seData;

    /* Get rid of the top element of the stack */
    STDFREE( stelem );

    return data;
}
void*
stkPeek(STACK stack)
{
    StackElement *	stelem;

    stelem = ST_TOP(stack);

    if (!stelem)
    	return NULL;

    return stelem->seData;
}
int stkSize(STACK stack) { return ((Stack*)stack)->stTotal; }
int stkEmpty(STACK stack) { return stkSize(stack) == 0; }

//----------------------------------------------------------------
// Statically-allocated memory manager
//
// by Eli Bendersky (eliben@gmail.com)
//
// This code is in the public domain.
//----------------------------------------------------------------
//apa modified for adding support to multi threads
#include <stdio.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <memmgr.h>
#include <malloc.h> //apa+++

/** mutex for thread to stop the threads hitting data at the same time. */
ins_mutex_t * mut = NULL; //apa+++

typedef ulong Align;

union mem_header_union
{
    struct
    {
        // Pointer to the next block in the free list
        //
        union mem_header_union* next;

        // Size of the block (in quantas of sizeof(mem_header_t))
        //
        ulong size;
    } s;

    // Used to align headers in memory to a boundary
    //
    Align align_dummy;
};

typedef union mem_header_union mem_header_t;

// Initial empty list
//
static mem_header_t base;

// Start of free list
//
static mem_header_t* freep = 0;

// Static pool for new allocations
//
//static byte pool[POOL_SIZE] = {0}; //apa---
static byte* pool = NULL; //apa+++
static ulong pool_free_pos = 0;


void memmgr_init()
{
    base.s.next = 0;
    base.s.size = 0;
    freep = 0;
    pool_free_pos = 0;

	mut = ins_mutex_new(); //apa+++

	pool = (byte*)malloc(POOL_SIZE); //apa+++

	memset(pool, 0, POOL_SIZE); //apa+++

	if(pool == NULL) //apa+++
	{
		printf("Failure in allocation of pool memory\n"); //apa+++
	}
}

//apa+++
void memmgr_terminate()
{
	ins_mutex_free(mut);
	free(pool);
}

void memmgr_print_stats()
{
    #ifdef DEBUG_MEMMGR_SUPPORT_STATS
    mem_header_t* p;

    printf("------ Memory manager stats ------\n\n");
    printf(    "Pool: free_pos = %lu (%lu bytes left)\n\n",
            pool_free_pos, POOL_SIZE - pool_free_pos);

    p = (mem_header_t*) pool;

    while (p < (mem_header_t*) (pool + pool_free_pos))
    {
        printf(    "  * Addr: %p; Size: %8lu\n",
                p, p->s.size);

        p += p->s.size;
    }

    printf("\nFree list:\n\n");

    if (freep)
    {
        p = freep;

        while (1)
        {
            printf(    "  * Addr: %p; Size: %8lu; Next: %p\n",
                    p, p->s.size, p->s.next);

            p = p->s.next;

            if (p == freep)
                break;
        }
    }
    else
    {
        printf("Empty\n");
    }

    printf("\n");
    #endif // DEBUG_MEMMGR_SUPPORT_STATS
}


static mem_header_t* get_mem_from_pool(ulong nquantas)
{
    ulong total_req_size;

    mem_header_t* h;

    if (nquantas < MIN_POOL_ALLOC_QUANTAS)
        nquantas = MIN_POOL_ALLOC_QUANTAS;

    total_req_size = nquantas * sizeof(mem_header_t);

    if (pool_free_pos + total_req_size <= POOL_SIZE)
    {
        h = (mem_header_t*) (pool + pool_free_pos);
        h->s.size = nquantas;
        memmgr_free((void*) (h + 1));
        pool_free_pos += total_req_size;
    }
    else
    {
        return 0;
    }

    return freep;
}


// Allocations are done in 'quantas' of header size.
// The search for a free block of adequate size begins at the point 'freep'
// where the last block was found.
// If a too-big block is found, it is split and the tail is returned (this
// way the header of the original needs only to have its size adjusted).
// The pointer returned to the user points to the free space within the block,
// which begins one quanta after the header.
//
void* memmgr_alloc(ulong nbytes)
{
    mem_header_t* p;
    mem_header_t* prevp;

    // Calculate how many quantas are required: we need enough to house all
    // the requested bytes, plus the header. The -1 and +1 are there to make sure
    // that if nbytes is a multiple of nquantas, we don't allocate too much
    //
    ulong nquantas = (nbytes + sizeof(mem_header_t) - 1) / sizeof(mem_header_t) + 1;

	//printf("Alloc from pool = %x in thread ID = %d\n", pool, GetCurrentThreadId());

    // First alloc call, and no free list yet ? Use 'base' for an initial
    // denegerate block of size 0, which points to itself
    //
    if ((prevp = freep) == 0)
    {
        base.s.next = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.next; ; prevp = p, p = p->s.next)
    {
        // big enough ?
        if (p->s.size >= nquantas)
        {
            // exactly ?
            if (p->s.size == nquantas)
            {
                // just eliminate this block from the free list by pointing
                // its prev's next to its next
                //
                prevp->s.next = p->s.next;
            }
            else // too big
            {
                p->s.size -= nquantas;
                p += p->s.size;
                p->s.size = nquantas;
            }

            freep = prevp;
            return (void*) (p + 1);
        }
        // Reached end of free list ?
        // Try to allocate the block from the pool. If that succeeds,
        // get_mem_from_pool adds the new block to the free list and
        // it will be found in the following iterations. If the call
        // to get_mem_from_pool doesn't succeed, we've run out of
        // memory
        //
        else if (p == freep)
        {
            if ((p = get_mem_from_pool(nquantas)) == 0)
            {
                //#ifdef DEBUG_MEMMGR_FATAL
                printf("!! Memory allocation failed !!\n");
                ExitProcess(0); //exit process
                //#endif
                return 0;
            }
        }
    }
}

//apa+++
void* memmgr_alloc_mt(ulong nbytes)
{
	void* p;
	ins_mutex_acquire(mut);//lock
	p = memmgr_alloc(nbytes);
	ins_mutex_release(mut); //unlock
	return p;
}


// Scans the free list, starting at freep, looking the the place to insert the
// free block. This is either between two existing blocks or at the end of the
// list. In any case, if the block being freed is adjacent to either neighbor,
// the adjacent blocks are combined.
//
void memmgr_free(void* ap)
{
    mem_header_t* block;
    mem_header_t* p;

	//printf("Free from pool = %x in thread ID = %d\n", pool, GetCurrentThreadId());

    // acquire pointer to block header
    block = ((mem_header_t*) ap) - 1;

    // Find the correct place to place the block in (the free list is sorted by
    // address, increasing order)
    //
    for (p = freep; !(block > p && block < p->s.next); p = p->s.next)
    {
        // Since the free list is circular, there is one link where a
        // higher-addressed block points to a lower-addressed block.
        // This condition checks if the block should be actually
        // inserted between them
        //
        if (p >= p->s.next && (block > p || block < p->s.next))
            break;
    }

    // Try to combine with the higher neighbor
    //
    if (block + block->s.size == p->s.next)
    {
        block->s.size += p->s.next->s.size;
        block->s.next = p->s.next->s.next;
    }
    else
    {
        block->s.next = p->s.next;
    }

    // Try to combine with the lower neighbor
    //
    if (p + p->s.size == block)
    {
        p->s.size += block->s.size;
        p->s.next = block->s.next;
    }
    else
    {
        p->s.next = block;
    }

    freep = p;
}

//apa+++
void memmgr_free_mt(void* ap)
{
	ins_mutex_acquire(mut);//lock
	memmgr_free(ap);
	ins_mutex_release(mut); //unlock
}

/////////apa+++ thread support//////////////
#include <malloc.h>
#if defined(USE_WIN32_THREADS)
void
ins_mutex_init(ins_mutex_t *m)
{
  InitializeCriticalSection(&m->mutex);
}
void
ins_mutex_uninit(ins_mutex_t *m)
{
  DeleteCriticalSection(&m->mutex);
}
void
ins_mutex_acquire(ins_mutex_t *m)
{
  //assert(m);
  EnterCriticalSection(&m->mutex);
}
void
ins_mutex_release(ins_mutex_t *m)
{
  LeaveCriticalSection(&m->mutex);
}
unsigned long
ins_get_thread_id(void)
{
  return (unsigned long)GetCurrentThreadId();
}
#elif defined(USE_PTHREADS)
/** A mutex attribute that we're going to use to tell pthreads that we want
 * "reentrant" mutexes (i.e., once we can re-lock if we're already holding
 * them.) */
static pthread_mutexattr_t attr_reentrant;
/** True iff we've called ins_threads_init() */
static int threads_initialized = 0;
/** Initialize <b>mutex</b> so it can be locked.  Every mutex must be set
 * up with ins_mutex_init() or ins_mutex_new(); not both. */
void
ins_mutex_init(ins_mutex_t *mutex)
{
  int err;
  if (PREDICT_UNLIKELY(!threads_initialized))
	ins_threads_init();
  err = pthread_mutex_init(&mutex->mutex, &attr_reentrant);
  if (PREDICT_UNLIKELY(err)) {
	log_err(LD_GENERAL, "Error %d creating a mutex.", err);
	ins_fragile_assert();
  }
}
/** Wait until <b>m</b> is free, then acquire it. */
void
ins_mutex_acquire(ins_mutex_t *m)
{
  int err;
  assert(m);
  err = pthread_mutex_lock(&m->mutex);
  if (PREDICT_UNLIKELY(err)) {
	log_err(LD_GENERAL, "Error %d locking a mutex.", err);
	ins_fragile_assert();
  }
}
/** Release the lock <b>m</b> so another thread can have it. */
void
ins_mutex_release(ins_mutex_t *m)
{
  int err;
  assert(m);
  err = pthread_mutex_unlock(&m->mutex);
  if (PREDICT_UNLIKELY(err)) {
	log_err(LD_GENERAL, "Error %d unlocking a mutex.", err);
	ins_fragile_assert();
  }
}
/** Clean up the mutex <b>m</b> so that it no longer uses any system
 * resources.  Does not free <b>m</b>.  This function must only be called on
 * mutexes from ins_mutex_init(). */
void
ins_mutex_uninit(ins_mutex_t *m)
{
  int err;
  assert(m);
  err = pthread_mutex_destroy(&m->mutex);
  if (PREDICT_UNLIKELY(err)) {
	log_err(LD_GENERAL, "Error %d destroying a mutex.", err);
	ins_fragile_assert();
  }
}
/** Return an integer representing this thread. */
unsigned long
ins_get_thread_id(void)
{
  union {
	pthread_t thr;
	unsigned long id;
  } r;
  r.thr = pthread_self();
  return r.id;
}
#endif

/** Return a newly allocated, ready-for-use mutex. */
ins_mutex_t *
ins_mutex_new(void)
{
  ins_mutex_t *m = (struct ins_mutex_t *)calloc(1,sizeof(ins_mutex_t));
  ins_mutex_init(m);
  return m;
}
/** Release all storage and system resources held by <b>m</b>. */
void
ins_mutex_free(ins_mutex_t *m)
{
  ins_mutex_uninit(m);
  free(m);
}


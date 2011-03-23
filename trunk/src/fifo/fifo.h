/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef FIFO__H
#define FIFO__H

#include <stdio.h>
#include "shmem.h"

#pragma warning (disable: 4291)

class fifo_obj { 
  protected: 
    class item { 
		public:
		#ifdef USE_BASED_POINTERS
		REF(item) next;
		#else
		item* next;
		#endif
		int   length;
		char  buffer[1]; 

		void* operator new(size_t fixed_size,shared_memory& shmem, size_t varying_size) 
		{
			return shmem.allocate(fixed_size + varying_size - 4);
		}

		void operator delete(void* p) 
		{
			shared_memory::deallocate(p);
		}

		void store(char* message, int message_length) 
		{
			length = message_length;
			next = NULL;
			memcpy(buffer, message, length);
		}
    };

    class header { 
		private:
		#ifdef USE_BASED_POINTERS
		REF(item) tail;
		REF(item) head;
		#else
		item* tail;
		item* head;
		#endif

		public:
		void* operator new(size_t size, shared_memory& shmem) 
		{
			return shmem.allocate(size);
		}

		void operator delete(void* p) 
		{
			shared_memory::deallocate(p);
		}

		header() 
		{
			tail = NULL;
			head = NULL;
		}

		bool enqueue(shared_memory& shmem, event& not_full,char* message, int message_length)
		{
			exclusive_lock xlock(shmem);
			// create object of varying size	
			item* ip = new (shmem, message_length) item;
			if (ip == NULL) 
			{ 
				not_full.reset();
				return false; // not enough space in storage
			}

			ip->store(message, message_length);

			if (head == NULL) 
			{ 
				head = tail = (REF(item))ip;
			} 
			else 
			{ 
				tail = tail->next = (REF(item))ip;
			}
			return true;
		}

		int dequeue(shared_memory& shmem, char* buf, int buf_size) 
		{ 
			exclusive_lock xlock(shmem);
			item* ip = head;
			if (ip == NULL) { // queue not empty
			return -1;
			} 
			head = head->next;
			int length = ip->length < buf_size ? ip->length : buf_size;
			memcpy(buf, ip->buffer, length);
			delete ip;
			return length;
		}
    };

    header*       root;
    semaphore     not_empty;
    event         not_full;
    shared_memory shmem;
    char*         name;
    size_t        max_size;

	public:
    bool open(char const* name, size_t max_size) 
	{ 
		shared_memory::status rc = shmem.open(NULL, name, max_size);
		if (rc != shared_memory::ok) 
		{ 
			return false;
		}

		root = (header*)shmem.get_root_object();

		if (root == NULL) 
		{ 
			exclusive_lock xlock(shmem);
			root = new (shmem) header;
			shmem.set_root_object(root);
		}

		size_t len = strlen(name);
		char* global_name = new char[len+5];
		strcpy(global_name, name);
		strcpy(global_name+len, ".put");

		if (!not_full.open(global_name)) 
		{ 	
			delete[] global_name;
			return false;
		}

		strcpy(global_name+len, ".get");

		if (!not_empty.open(global_name)) 
		{ 
			delete[] global_name;
			return false;
		}

		delete[] global_name;
		return true;
    }
    void close() 
	{ 
		shmem.close();
		not_empty.close();
		not_full.close();
    }
    
    void put(char* message, int length) 
	{ 
		while (!root->enqueue(shmem, not_full, message, length)) 
		{ 
			not_full.wait();
		}
		not_empty.signal();
    }

    int get(char* buf, int buf_size, unsigned msec) 
	{ 
		if (not_empty.wait(msec)) 
		{
			int len = root->dequeue(shmem, buf, buf_size);
			assert(len >= 0);
			not_full.signal();
			return len;
		}
		return -1;
    }
};

#endif //#define FIFO__H
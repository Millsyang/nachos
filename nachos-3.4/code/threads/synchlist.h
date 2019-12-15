// synchlist.h 
//	Data structures for synchronized access to a list.
//
//	Implemented by surrounding the List abstraction
//	with synchronization routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYNCHLIST_H
#define SYNCHLIST_H

#include "copyright.h"
#include "list.h"
#include "synch.h"

//Lab 4:stimulate producer-consumer problem
#define BUFFER_SIZE 10

// The following class defines a "synchronized list" -- a list for which:
// these constraints hold:
//	1. Threads trying to remove an item from a list will
//	wait until the list has an element on it.
//	2. One thread at a time can access list data structures

//Lab4:product struct
typedef struct{
  int value;
}product;

class SynchList {
  public:
    SynchList();		// initialize a synchronized list
    ~SynchList();		// de-allocate a synchronized list

    void Append(void *item);	// append item to the end of the list,
				// and wake up any thread waiting in remove
    void *Remove();		// remove the first item from the front of
				// the list, waiting if the list is empty
				// apply function to every item in the list
    void Mapcar(VoidFunctionPtr func);

  private:
    List *list;			// the unsynchronized list
    Lock *lock;			// enforce mutual exclusive access to the list
    Condition *listEmpty;	// wait in Remove if the list is empty
};


//Lab4:add for stimulate producer-consumer problem
class buffer{
  public:
    buffer(){
      fullCount = new Semaphore("Full Count", 0);
      emptyCount = new Semaphore("Empty Count", BUFFER_SIZE);
      buffer_mutex = new Lock("Buffer mutex");
      count = 0;//product:0 at first
    }

    ~buffer(){
      delete fullCount;
      delete emptyCount;
      delete buffer_mutex;
      //array need delete?
      // delete list;
    }

    void putItemIntoBuffer(product* item){
      emptyCount->P();
      buffer_mutex->Acquire();
      list[count++] = *item;
      buffer_mutex->Release();
      fullCount->V();
    }

    product* removeItemFromBuffer(){
      product* item;
      fullCount->P();
      buffer_mutex->Acquire();
      item = &list[count-1];
      count--;
      buffer_mutex->Release();
      emptyCount->V();
      return item;
    }

    void printBuffer(){
      printf("Buffer: [%d,%d", BUFFER_SIZE, count);
      int i;
      for (i = 0; i < count; i++) {
        printf("%d, ", list[i].value);
      }
      for (;i < BUFFER_SIZE; i++) {
        printf("__, ");
      }
      printf("]\n");
    }
  private:
    int count;
    Lock* buffer_mutex;
    Semaphore* fullCount;
    Semaphore* emptyCount;
    product list[BUFFER_SIZE];
};

class buffer2{
  public:
    buffer2(){ //structor methods of buffer2
      buffer_mutex = new Lock("buffer2_mutex");
      full = new Condition("Full buffer exist");
      empty = new Condition("empty buffer exist");
      count = 0;
    }
    
    ~buffer2(){ //destructor methods of buffer2
      delete buffer_mutex;
      delete full;
      delete empty;
    }

    void putItemIntoBuffer(product *item){
      buffer_mutex->Acquire();
      if(count == BUFFER_SIZE) empty->Wait(buffer_mutex);
      list[count++] = *item;
      if(count == 1) full->Signal(buffer_mutex);
      buffer_mutex->Release();
    }

    product* removeItemFromBuffer(){
      product* item;
      buffer_mutex->Acquire();
      if(!count) full->Wait(buffer_mutex);
      item = &list[count-1];
      count--;
      if(count == BUFFER_SIZE-1) empty->Signal(buffer_mutex); 
      buffer_mutex->Release();
      return item;
    }

    void printBuffer(){
      printf("Buffer: [%d,%d", BUFFER_SIZE, count);
      int i;
      for (i = 0; i < count; i++) {
        printf("%d, ", list[i].value);
      }
      for (;i < BUFFER_SIZE; i++) {
        printf("__, ");
      }
      printf("]\n");
    }

  private:
    int count;    //product's number
    Lock* buffer_mutex;  //lock for mutual exclusion access
    Condition* full;     //when count=0,wait full
    Condition* empty;    //when count=BUFFER_SIZe,wait empty
    product list[BUFFER_SIZE];
};
#endif // SYNCHLIST_H

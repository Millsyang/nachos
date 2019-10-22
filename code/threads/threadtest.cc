// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread name %s userID %d threadID %d priority %d looped %d times\n", currentThread->getName(),which,currentThread->getThreadID(), currentThread->getPriority(),num);
    }
}


//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------
void
ThreadTest1()
{
	DEBUG('t', "Entering ThreadTest1");

	Thread* t = new Thread("forked thread");

	t->Fork(SimpleThread, currentThread->getUserID());
	//why execute before t->Fork?
	SimpleThread(currentThread->getUserID());
}
//----------------------------------------------------------------------
// ThreadTest2
// Test exercise3
//----------------------------------------------------------------------
void
ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");

    Thread* t1 = new Thread("forked thread1",3);
	Thread* t2 = new Thread("forked thread2",2);
	Thread* t3 = new Thread("forked thread3",1);

    t1->Fork(SimpleThread, t1->getThreadID());
    t2->Fork(SimpleThread, t2->getThreadID());
    t3->Fork(SimpleThread, t3->getThreadID());
	SimpleThread(0);
}
//----------------------------------------------------------------------
// ThreadLimit
// Test exercise4
//----------------------------------------------------------------------
void 
ThreadLimit()
{
	DEBUG('t', "Test limit of threadnum");
	for (int i = 0; i <= MaxThreadNum; i++)
	{
		Thread* t = new Thread("test thread");
		printf("*** thread name %s userID %d threadID %d\n", t->getName(), t->getUserID(), t->getThreadID());
	}
}
//----------------------------------------------------------------------
// ThreadTest3
// check options 'TS' applicable or not
//----------------------------------------------------------------------
void 
ThreadTest3()
{
	DEBUG('t', "check options 'TS' applicable or not");
	Thread* t1 = new Thread("test thread1");
	Thread* t2 = new Thread("test thread2");
	Thread* t3 = new Thread("test thread3");

	t1->Fork(displayThreadStatus,t1->getThreadID());
	t2->Fork(displayThreadStatus,t2->getThreadID());
	t3->Fork(displayThreadStatus,t3->getThreadID());
}
//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
	case 2:
	ThreadTest2();
	break;
	case 3:
	ThreadLimit();
	break;
	case 4:
	ThreadTest3();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}


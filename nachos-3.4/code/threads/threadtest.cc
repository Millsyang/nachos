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

//Lab4
//----------------------------------------------------------------------
// Produce Item
//  Generate product with value
//----------------------------------------------------------------------
product*
produceItem(int value)
{
    printf("Producing item with value %d!!\n", value);
    product item;
    item.value = value;
    return &item;
}

//----------------------------------------------------------------------
// Consume Item
//  Delete product
//----------------------------------------------------------------------
void
consumeItem(product* item)
{
    printf("Consuming item with value %d!!\n", item->value);
}

//----------------------------------------------------------------------
// Producer
//  generate data, put it into the container, and start again. 
//----------------------------------------------------------------------
void
ProducerThread(int iterNum)
{
    for (int i = 0; i < iterNum; i++) {
        printf("## %s ##: ", currentThread->getName());
        product* item = produceItem(i);
        container->putItemIntoBuffer(item);

        interrupt->OneTick();//make sure system time move forward
    }
}

//----------------------------------------------------------------------
// Consumer
//  consuming the data, one piece at a time.
//----------------------------------------------------------------------
void
ConsumerThread(int iterNum)
{
    for (int i = 0; i < iterNum; i++) {
        printf("$$ %s $$: \n", currentThread->getName());
        product* item = container->removeItemFromBuffer();
        consumeItem(item);

        interrupt->OneTick();
    }
}

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
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
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

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// Lab1Exercise3Thread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//---------------------------------------------------------------------

void
Lab1Exercise3Thread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
    printf("*** thread %d (uid=%d, tid=%d) looped %d times\n", which, currentThread->getUserID(), currentThread->getThreadID(), num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// Lab1 Exercise3
// 	Create multi-threads and show its uid and tid
//----------------------------------------------------------------------

void
Lab1Exercise3()
{
    DEBUG('t', "Entering Lab1Exercise3");

    const int max_threads = 5;
    const int test_uid = 666;

    for (int i = 0; i < max_threads; i++) {
        // Generate a Thread object
        Thread *t = new Thread("forked thread");
        t->setUserID(test_uid); // set uid
        // Define a new thread's function and its parameter
        t->Fork(Lab1Exercise3Thread, t->getThreadID());
    }

    Lab1Exercise3Thread(0);
}

//----------------------------------------------------------------------
// TS command
// 	Showing current threads' status (like ps in Linux)
//----------------------------------------------------------------------
void TS()
{
    DEBUG('t', "Entering TS");

    const char* TStoString[] = {"JUST_CREATED", "RUNNING", "READY", "BLOCKED"};

    printf("UID\tTID\tNAME\tSTATUS\n");
    for (int i = 0; i < MaxThreadNum; i++) { // check pid flag
        if (threadFlag[i]) {
            printf("%d\t%d\t%s\t%s\n", thread_pointer[i]->getUserID(), thread_pointer[i]->getThreadID(), thread_pointer[i]->getName(), TStoString[thread_pointer[i]->getThreadStatus()]);
        }
    }
}

//----------------------------------------------------------------------
// CustomThreadFunc
// a function to change current thread's status
// "which" is simply a number identifying the operation to do on current thread
//----------------------------------------------------------------------
void
CustomThreadFunc(int which)
{
    printf("*** current thread (uid=%d, tid=%d, pri=%d name=%s) => ", currentThread->getUserID(), currentThread->getThreadID(), currentThread->getPriority(), currentThread->getName());
    IntStatus oldLevel; // for case 1 sleep (avoid cross initialization problem of switch case)
    //0 for interrupt off,1 for interrupt on
    switch (which)
    {
        case 0:
            printf("Yield\n");
            scheduler->Print();
            printf("\n\n");
            currentThread->Yield();
            break;
        case 1:
            printf("Sleep\n");
            oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
            currentThread->Sleep();
            (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
            break;
        case 2:
            printf("Finish\n");
            currentThread->Finish();
            break;
        default:
            printf("Yield (default)\n");
            currentThread->Yield();
            break;
    }
}

//----------------------------------------------------------------------
// Lab1 Exercise4-2
// 	Create some threads and use TS to show the status
//----------------------------------------------------------------------
void
Lab1Exercise4_2()
{
    DEBUG('t', "Entering Lab1Exercise4_2");

    Thread *t1 = new Thread("fork 1");
    Thread *t2 = new Thread("fork 2");
    Thread *t3 = new Thread("fork 3");

    t1->Fork(CustomThreadFunc, 0);
    t2->Fork(CustomThreadFunc, 1);
    t3->Fork(CustomThreadFunc, 2);

    Thread *t4 = new Thread("fork 4");

    CustomThreadFunc(0); // Yield the current thread (i.e. main which is defined in system.cc)

    printf("--- Calling TS command ---\n");
    TS();
    printf("--- End of TS command ---\n");
}

//----------------------------------------------------------------------
// Lab2 Exercise3-1
// 	Fork some Thread with different ways to initial the priority
//----------------------------------------------------------------------
void
Lab2Exercise3_1()
{
    DEBUG('t', "Entering Lab2Exercise3_1");

    Thread *t1 = new Thread("with p",741);

    Thread *t2 = new Thread("set p");
    t2->setPriority(100);

    Thread *t3 = new Thread("no p");

    t1->Fork(CustomThreadFunc, 0);
    t2->Fork(CustomThreadFunc, 0);
    t3->Fork(CustomThreadFunc, 0);

    CustomThreadFunc(0); // Yield the current thread

    printf("--- Calling TS command ---\n");
    TS();
    printf("--- End of TS command ---\n");
}

//----------------------------------------------------------------------
//  Lab2 Exercise3-2
// 	Fork some Thread with different priority
//  and observe if the lower one will take over the CPU
//----------------------------------------------------------------------
void
Lab2Exercise3_2()
{
    DEBUG('t', "Entering Lab2Exercise3_2");

    Thread *t1 = new Thread("low", 1);
    Thread *t2 = new Thread("high", 10);
    // The lowest one will be put in front of the list
    // due to SortedInsert() in list.cc
    Thread *t3 = new Thread("mid", 5);

    t1->Fork(CustomThreadFunc, 0);
    t2->Fork(CustomThreadFunc, 0);
    t3->Fork(CustomThreadFunc, 0);

    CustomThreadFunc(0); // Yield the current thread

    // Because the main() Thread has priority 0
    // Then any process yield will make main keep running
    // Since 0 is the lowest number and it will be in front of the readyList
    // So the TS command will be called right after the first Yield()
    printf("--- Calling TS command ---\n");
    TS();
    printf("--- End of TS command ---\n\n");
}

//----------------------------------------------------------------------
// Lab4 Exercise 4 Producer-consumer problem (Bounded-buffer problem)
//  The problem describes two processes, the producer and the consumer,
//  who share a common, fixed-size buffer used as a queue.
//  The producer's job is to generate data, put it into the buffer,
//  and start again. 
//  At the same time, the consumer is consuming the data
//  (i.e., removing it from the buffer), one piece at a time.
//  The problem is to make sure that the producer won't try to add data
//  into the buffer if it's full and that the consumer won't try to
//  remove data from an empty buffer.
//----------------------------------------------------------------------
void
Lab4Producer_Consumer()
{
    DEBUG('t', "Entering Lab4ProducerConsumer");

    container = new buffer2();

    Thread *producer1 = new Thread("Producer 1");
    Thread *producer2 = new Thread("Producer 2");
    Thread *consumer1 = new Thread("Consumer 1");
    Thread *consumer2 = new Thread("Consumer 2");

    producer1->Fork(ProducerThread, 7);
    consumer1->Fork(ConsumerThread, 5);
    consumer2->Fork(ConsumerThread, 8);
    producer2->Fork(ProducerThread, 6);

    currentThread->Yield(); // Yield the main thread
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
    Lab1Exercise3();
	break;
    case 3:
    Lab1Exercise4_2();
    break;
    case 4:
    Lab2Exercise3_1();
    break;
    case 5:
    Lab2Exercise3_2();
    break;
    case 6:
    Lab4Producer_Consumer();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}


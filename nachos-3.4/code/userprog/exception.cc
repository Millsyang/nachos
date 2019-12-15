// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//Lab3:For handle tlb miss
// int TLBreplaceIdx = 0;

//Lab6:Helper function to get file name using ReadMem for Create and Open syscall
char* getFileNameFromAddress(int address);
void ExecSyscallHandler();
void ForkSyscallHandler();

//-----------------------------------------------------------------
// Lab3:FIFO swap
//-----------------------------------------------------------------
void
TLBAlgoFIFO(TranslationEntry page)
{
    int TLBreplaceIdx = -1;
    // Find the empty entry
    for (int i = 0; i < TLBSize; i++) {
        if (machine->tlb[i].valid == FALSE) {
            TLBreplaceIdx = i;
            break;
        }
    }
    // If full then move everything forward and remove the last one
    if (TLBreplaceIdx == -1) {
        TLBreplaceIdx = TLBSize - 1;
        for (int i = 0; i < TLBSize - 1; i++) {
            machine->tlb[i] = machine->tlb[i+1];
        }
    }
    // Update TLB
    machine->tlb[TLBreplaceIdx] = page;
}


//----------------------------------------------------------------------
// NaivePageReplacement
//  1. Find an non-dirty page frame to replace.
//  2. If not found, then replace a dirty page and write back to disk.
//  3. Return the page frame number when founded or after replacement.
//----------------------------------------------------------------------
int
NaivePageReplacement(int vpn)
{
    int pageFrame = -1;
    for (int temp_vpn = 0; temp_vpn < machine->pageTableSize, temp_vpn != vpn; temp_vpn++) {
        if (machine->pageTable[temp_vpn].valid) {
            if (!machine->pageTable[temp_vpn].dirty) {
                pageFrame = machine->pageTable[temp_vpn].physicalPage;
                break;
            }
        }
    }
    if (pageFrame == -1) { // No non-dirty entry
        for (int replaced_vpn = 0; replaced_vpn < machine->pageTableSize, replaced_vpn != vpn; replaced_vpn++) {
            if (machine->pageTable[replaced_vpn].valid) {
                machine->pageTable[replaced_vpn].valid = FALSE;
                pageFrame = machine->pageTable[replaced_vpn].physicalPage;

                // Store the page back to disk
                OpenFile *vm = fileSystem->Open("VirtualMemory");
                ASSERT(vm != NULL);
                vm->WriteAt(&(machine->mainMemory[pageFrame*PageSize]), PageSize, replaced_vpn*PageSize);
                delete vm; // close file
                break;
            }
        }
    }
    return pageFrame;
}

//--------------------------------------------------------------------------
//Lab3 PageFaultHandler
// 	1. Find an empty space in memory
//  2. Load the page frame from disk to memory
//      * If memory out of space then find a page to replace
//          * If all pages are dirty, then it need to write back to disk.
//--------------------------------------------------------------------------
TranslationEntry
PageFaultHandler(int vpn)
{
    // Get a Memory space (page frame) to allocate
    int pageFrame = machine->allocateFrame(); // ppn
    if (pageFrame == -1) { // Need page replacement
        pageFrame = NaivePageReplacement(vpn);
    }
    machine->pageTable[vpn].physicalPage = pageFrame;

    // Load the Page from virtual memory
    DEBUG('a', "Demand paging: loading page from virtual memory!\n");
    OpenFile *vm = fileSystem->Open("VirtualMemory"); // This file is created in userprog/addrspace.cc
    ASSERT(vm != NULL);
    vm->ReadAt(&(machine->mainMemory[pageFrame*PageSize]), PageSize, vpn*PageSize);
    delete vm; // close the file

    // Set the page attributes
    machine->pageTable[vpn].valid = TRUE;
    machine->pageTable[vpn].use = FALSE;
    machine->pageTable[vpn].dirty = FALSE;
    machine->pageTable[vpn].readOnly = FALSE;

    currentThread->space->PrintState(); // debug with -d M to show bitmap
}


//-----------------------------------------------------------------------------------
// Lab3:TLBMissHandler
// Circly put badVddr into tlb[0] or tlb[1]
//-----------------------------------------------------------------------------------
void
TLBMissHandler(int virtAddr)
{
    unsigned int vpn;

    vpn = (unsigned) virtAddr / PageSize;
    // Find the Page
    TranslationEntry page = machine->pageTable[vpn];
    if (!page.valid) { // Lab4: Demand paging
        DEBUG('m', "\t=> Page miss\n");
        page = PageFaultHandler(vpn);
    }

    // ONLY USE FOR TESTING Lab4 Exercise2
    // i.e. assume TLBSize = 2
    // machine->tlb[TLBreplaceIdx] = machine->pageTable[vpn];
    // TLBreplaceIdx = TLBreplaceIdx ? 0 : 1;
    TLBAlgoFIFO(machine->pageTable[vpn]);
}

//----------------------------------------------------------------------
// IncrementPCRegs
// 	Because when Nachos cause the exception. The PC won't increment
//  (i.e. PC+4) in Machine::OneInstruction in machine/mipssim.cc.
//  Thus, when invoking a system call, we need to advance the program
//  counter. Or it will cause the infinity loop.
//----------------------------------------------------------------------
void IncrementPCRegs(void) {
    // Debug usage
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));

    // Advance program counter
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    //Lab3:page fault handling
    if(which == PageFaultException){
        if (machine->tlb == NULL) { // linear page table page fault
            DEBUG('m', "=> Page table page fault.\n");
            // In current Nachos this shouldn't happen
            // because physical page frame == virtual page number
            // (can be found in AddrSpace::AddrSpace in userprog/addrspace.cc)
            // On the other hand, in our Lab we won't use linear page table at all
            ASSERT(FALSE);
        } else { // TLB miss (no TLB entry)
            // Lab4 Exercise2
            DEBUG('m', "=> TLB miss (no TLB entry)\n");
            int BadVAddr = machine->ReadRegister(BadVAddrReg); // The failing virtual address on an exception
            TLBMissHandler(BadVAddr);
        }
        return;
    }

    int type = machine->ReadRegister(2);

    if(which == SyscallException){
        if(type == SC_Halt){
            DEBUG('a', "Shutdown, initiated by user program.\n");
   	        interrupt->Halt();      
        }else if(type == SC_Exit || type == SC_Exec){
            AddressSpaceControlHandler(type);
        }else if (type == SC_Create || type == SC_Open || type == SC_Write || type == SC_Read || type == SC_Close) {
            //Lab6:File System System Calls
            FileSystemHandler(type);
        }else if (type == SC_Fork || type == SC_Yield){
            UserLevelThreadsHandler(type);
        }
        if(type != SC_Yield) IncrementPCRegs();
        return;
    }

	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
}

void AddressSpaceControlHandler(int type)
{
    if (type == SC_Exit) {

        //PrintTLBStatus(); // TLB debug usage

        int status = machine->ReadRegister(4); // r4: first arguments to functions

        // currentThread->setExitStatus(status);
        if (status == 0) {
            DEBUG('S', "User program exit normally. (status 0)\n");
        } else {
            DEBUG('S', "User program exit with status %d\n", status);
        }

        // TODO: release children

        #ifdef USER_PROGRAM
        if (currentThread->space != NULL) {
            #ifdef USE_BITMAP
            machine->freeMem(); // ONLY USE FOR TEST Lab4 Exercise4
            DEBUG('M',"BitMap after freed %d.\n",machine->bitmap);
            #endif
            delete currentThread->space;
            currentThread->space = NULL;
        }
        #endif
        // TODO: if it has parent, then set this to zombie and signal
        currentThread->Finish();
    }else if(type == SC_Exec){
        DEBUG('S', "Syscall: Exec\n");
        ExecSyscallHandler();
    }else if(type == SC_Join){
        DEBUG('S',"Syscall: Join.\n");
        int threadid = machine->ReadRegister(4);
        while (threadFlag[threadid])
            currentThread->Yield();
    }
}

#define FileNameMaxLength 9 //originally defined in filesys/directory.cc
/**
 * FileSystemHandler 
 * Syscall related to Filesys
 */
void FileSystemHandler(int type){
    if(type == SC_Create){//interface: void Create(char* name)
        int address = machine->ReadRegister(4); // memory starting position
        DEBUG('S', "Received Create syscall (r4 = %d): ",address);
        char* name = getFileNameFromAddress(address);

        bool success = fileSystem->Create(name, 0); // initial file length set 0

        if(success){
            DEBUG('S',"file \"%s\" created.\n",name);
        }else{
            DEBUG('S',"file \"%s\" fail to create.\n",name);
        }
        // machine->WriteRegister(2, (int)success); // return result
    }else if(type == SC_Open){ //interface:openFileId *Open(char* name)
        int address = machine->ReadRegister(4); // memory starting position
        DEBUG('S',"Received Open syscall (r4 = %d): ",address);
        char* name = getFileNameFromAddress(address);

        OpenFile *openFile = fileSystem->Open(name);

        DEBUG('S', "File \"%s\" opened.\n", name);
        machine->WriteRegister(2, (OpenFileId)openFile); // return result
    }else if(type == SC_Close){
        OpenFileId id = machine->ReadRegister(4); // OpenFile object id
        DEBUG('S',"Received Close syscall (r4 = %d): ", id);

        OpenFile* openFile = (OpenFile*)id; // transfer id back to OpenFile
        delete openFile; // release the file

        DEBUG('S',"File has closed.\n");
    }else if(type == SC_Read){//interface:int Read(char* buffer,int size,openFileId id)
        int address = machine->ReadRegister(4); // memory starting position
        int size = machine->ReadRegister(5); // read "size" bytes
        OpenFileId id = machine->ReadRegister(6); // OpenFile object id
        DEBUG('S',"Received Read syscall (r4 = %d, r5 = %d, r6 = %d): ", address, size, id);

        OpenFile* openFile = (OpenFile*)id; // transfer id back to OpenFile
        char* buffer = new char[size];
        int numBytes = openFile->Read(buffer, size);
        for (int i = 0; i < numBytes; i++) { // each time write one byte
            bool success = machine->WriteMem(address + i, 1, (int)buffer[i]);
            if (!success) {
                i--;
            }
        }
        DEBUG('S', "Read %d bytes into buffer.\n",numBytes);
        machine->WriteRegister(2, numBytes); // Return the number of bytes actually read
    }else if(type == SC_Write){//interface:void Write(char* buffer,int size,openFileId id)
        int address = machine->ReadRegister(4); // memory starting position
        int size = machine->ReadRegister(5); // read "size" bytes
        OpenFileId id = machine->ReadRegister(6); // OpenFile object id
        DEBUG('S',"Received Write syscall (r4 = %d, r5 = %d, r6 = %d): ", address, size, id);

        char* buffer = new char[size];
        for (int i = 0; i < size; i++) { // each time write one byte
            bool success = machine->ReadMem(address + i, 1, (int*)&buffer[i]);
            if (!success) {
                i--;
            }
        }
        OpenFile* openFile = (OpenFile*)id; // transfer id back to OpenFile
        int numBytes = openFile->Write(buffer, size);

        DEBUG('S',"Write %d bytes into file.\n", numBytes);
        machine->WriteRegister(2, numBytes); // Return the number of bytes actually write
    }
}

void UserLevelThreadsHandler(int type){
    if(type == SC_Fork){
        ForkSyscallHandler();
    }else if(type == SC_Yield){
        DEBUG('S',"Syscall:Yield.\n");
        IncrementPCRegs();
        currentThread->Yield();
    }
}

char* getFileNameFromAddress(int address) {
    int position = 0;
    int data;
    char name[FileNameMaxLength + 1];
    do {
        // each time read one byte
        bool success = machine->ReadMem(address + position, 1, &data);
        if(!success){
            printf("Fail to read memory in Create syscall.\n");
            ASSERT(FALSE);
        }
        name[position++] = (char)data;
        
        if(position > FileNameMaxLength){
            printf("Filename length too long.\n");
            ASSERT(FALSE)
        }
    } while(data != '\0');
    name[position] = '\0';
    return name;
}

// void ExecRoutine(int arg) {
//     currentThread->space->InitRegisters();
//     currentThread->space->RestoreState();
//     Machine *p = (Machine *)arg;
//     p->Run();
// }

// void ExecSyscallHandler() {
//     currentThread->SaveUserState();
//     // First, get the length of filename
//     int fileNameBase = machine->ReadRegister(4);
//     int value;
//     int length = 0;
//     do {
//         machine->ReadMem(fileNameBase++, 1, &value);
//         length++;
//     } while(value != '\0');

//     // Copy filename
//     char *fileName = new char[length];
//     fileNameBase -= length; length--;
//     for(int i = 0; i < length; i++) {
//         machine->ReadMem(fileNameBase++, 1, &value);
//         fileName[i] = char(value);
//     }
//     fileName[length] = '\0';
//     DEBUG('a', "Executable file name: %s\n", fileName);

//     OpenFile *executable = fileSystem->Open(fileName);

//     if(executable != NULL)
//         DEBUG('a', "Open file %s done\n", fileName);
//     else {
//         DEBUG('a', "Can not open file %s\n", fileName);
//         machine->WriteRegister(2, (int)executable);
//         return;
//     }

//     // Create an address space and a new thread
//     AddrSpace *addrSpace = new AddrSpace(executable);
//     Thread *forked = new Thread(fileName);
//     forked->space = addrSpace;

//     // Run user program
//     forked->Fork(ExecRoutine, (int)machine);
//     DEBUG('t', "Exec done\n");
//     currentThread->RestoreUserState();
//     machine->WriteRegister(2, (int)addrSpace);
// }

/**
 * imitate function startProcess
 * @param address point name of file to be executed.
 */
void exec_func(int address){
    char name[FileNameMaxLength+1];
    int pos =0;
    int data;
    while(1){
        machine->ReadMem(address+pos,1,&data);
        if(data == 0){
            name[pos] = '\0';
            break;
        }
        name[pos++] = char(data);
    }
    DEBUG('S',"Execute file %s.\n",name);
    OpenFile *executable = fileSystem->Open(name);
    AddrSpace *space = new AddrSpace(executable);
    currentThread->space = space;
    delete executable;
    space->InitRegisters();
    space->RestoreState();
    machine->Run();
    ASSERT(FALSE);
}

/**
 * ExecSyscallHandler
 * 
 */
void ExecSyscallHandler(){
    int address = machine->ReadRegister(4);
    Thread* newthread = new Thread("second thread");
    newthread->Fork(exec_func,address);
    machine->WriteRegister(2,newthread->getThreadID());
}

void ForkSyscallHandler(){
    DEBUG('S',"Syscall Fork.\n");
    // int function_pc = machine->ReadRegister(4);
    // OpenFile *executable = fileSystem->Open(currentThread->getFilename());
}

/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int
main()
{   
    int id = Exec("matmult");
    Join(id);
    // char a[8];
    // int exitCode = -1;
    // SpaceId sp;

    // a[0] = 'm';
    // a[1] = 'a';
    // a[2] = 't';
    // a[3] = 'm';
    // a[4] = 'u';
    // a[5] = 'l';
    // a[6] = 't';
    // a[7] = '\0';
    // sp = Exec(a);
    // Join(sp);
    // char a[5];
    // int exitCode = -1;
    // SpaceId sp;

    // a[0] = 'h';
    // a[1] = 'a';
    // a[2] = 'l';
    // a[3] = 't';
    // a[4] = '\0';

    // sp = Exec(a);
    // // Yield();
    // // exitCode = Join(sp);
    // Exit(exitCode);
    // char a[8];
    // int exitCode = -1;
    // SpaceId sp;

    // a[0] = 'm';
    // a[1] = 'a';
    // a[2] = 't';
    // a[3] = 'm';
    // a[4] = 'u';
    // a[5] = 'l';
    // a[6] = 't';
    // a[7] = '\0';

    // sp = Exec(a);

    // // exitCode = Join(sp);
    // Exit(exitCode);

    // char data[9]; // as file name and content
    // char buffer[9];
    // OpenFileId fid_write;
    // OpenFileId fid_read;
    // int numBytes;

    // data[0] = 't';
    // data[1] = 'e';
    // data[2] = 's';
    // data[3] = 't';
    // data[4] = '.';
    // data[5] = 't';
    // data[6] = 'x';
    // data[7] = 't';
    // data[8] = '\0';

    // Create(data);

    // fid_write = Open(data);
    // fid_read = Open(data);

    // Write(data, 8, fid_write);

    // numBytes = Read(buffer, 8, fid_read);

    // Close(fid_write);
    // Close(fid_read);

    // Exit(numBytes);
    // Halt();
    /* not reached */
}

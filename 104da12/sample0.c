#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>

#include "acceslib.h"


unsigned int baseAddr;
int fd;

unsigned int ask_for_baseAddr(unsigned int old);
int kbhit(void);
int open_dev_file();
void CtrMode(unsigned addr, char cntr, char mode);
void CtrLoad(unsigned addr ,int c,int val);
unsigned CtrRead(unsigned addr , unsigned int c);
void intro ();

int main (void)
{
	__u16 BUFFER[30000];
	int count;

	intro();

	baseAddr = ask_for_baseAddr(0x300);

	fd = open_dev_file();

	    /*****************************************************
    ** This for loop will fill the buffer with the following
    ** three wave forms and rescales them to have the same period:
    ** 1) sin(x^2) from x = -3.3 to 3.3
    ** 2) cos(x^2) from x = -3.0 to 3.0
    ** 3) sin(x^2) * cos (x^2) from x = -1.9 to 1.9
    *****************************************************/



    for (count = 0; count < 10000; count++)
    {
        BUFFER[(count * 3)] = (__u16)(sin(pow(-3.3 + (0.00066 * count), 2)) * 2047.0 + 2048.0);
        BUFFER[(count * 3)] &= 0x0fff; //need to be certain that the control bits are what we want


        BUFFER[((count * 3) + 1)] = (__u16)(cos(pow(-3.0 + (.0006 * count), 2)) * 2047.0 + 2048.0);
        BUFFER[((count * 3) + 1)] &= 0x0fff;

        BUFFER[((count * 3) + 2)] = (__u16)(sin(pow(-1.9 + (.00038 * count), 2)) * cos(pow(-1.9 + (.00038 * count), 2)) * 2047.0 + 2048.0);
        BUFFER[((count * 3) + 2)] &= 0x0fff; //let the card know that this is the end of this scan
        BUFFER[((count * 3) + 2)] |= 0x2000;

    }

    BUFFER[29999] |= 0x1000; //this is the end of the stream and the card needs to loop


    //now we need to fill the SRAM on the card;
    outportb(fd, baseAddr + 0x1a, 0); //we will only be writing to the first 30k addresses
                             //so we get to leave bit 16 of the SRAM addr at 0

    for (count = 0; count < 30000; count++)
    {
        outport(fd, baseAddr + 0x18, (count * 2)); //set the address we are going to write to
        outport(fd, baseAddr + 0x1c, BUFFER[count]); //write the value for that address
    }


    CtrMode(baseAddr + 0x14, 1, 2); //set counter 1 to mode 2
    CtrMode(baseAddr + 0x14, 2, 2); //set counter 2 to mode 2

    CtrLoad(baseAddr + 0x14, 1, 5); //load counter 1 to 5 ticks
    CtrLoad(baseAddr + 0x14, 2, 10); //load counter 2 to 10 ticks



    outportb(fd, baseAddr + 0x10, 0x41); //tell the card to start and enable the
                                 //ARB


    puts("\nPress <enter> to stop");
	
	 fflush(stdout);

    do
    {
        NULL;
    }while (!kbhit());

    outportb(fd, baseAddr + 0x10, 0); //tell the card to stop




	close(fd);
} 

void intro ()
{
    puts("This program will output three wave forms from the 104-DA12-8.");
    puts("The wave forms will be output on DACS 0 - 2.");
    puts("The wave forms will stop when the program exits.");
}

unsigned int ask_for_baseAddr(unsigned int old)
{
	unsigned int newone;
	char buf[1024];
	int count;
	int done = 0;

	do
	{
		printf("Please enter the baseAddr address or press enter for %X\n=>", old);

		count = 0;

		do
		{
			buf[count] = getchar();
			if ((isxdigit(buf[count]) == 0) && (buf[count] != '\n'))
			{
				buf[0] = 'x';
			}
			count++;
		}while ((buf[count - 1] != '\n') && (count < 1024));


		if (count == 1024)
		{
			count--;
			while (buf[count] != '\n')
			{
				buf[count] = getchar();
			}
		}
		else if (buf[0] == '\n')
		{
			newone = old;
			done = 1;
		}
		else if (sscanf(buf, "%x", &newone) == 1)
		{
			done = 1;
		}


	}while (!done);

	return newone;

}

//returns 1 for character waiting 0 for not

int kbhit(void)
{
  struct timeval tv;
  fd_set read_fd;

  /* Do not wait at all, not even a microsecond */
  tv.tv_sec=0;
  tv.tv_usec=0;

  /* Must be done first to initialize read_fd */
  FD_ZERO(&read_fd);

  /* Makes select() ask if input is ready:
   * 0 is the file descriptor for stdin    */
  FD_SET(0,&read_fd);

  /* The first parameter is the number of the
   * largest file descriptor to check + 1. */
  if(select(1, &read_fd, NULL, NULL, &tv) == -1)
    return 0;   /* An error occured */

  /*    read_fd now holds a bit map of files that are
   * readable. We test the entry for the standard
   * input (file 0). */
  if(FD_ISSET(0,&read_fd))
    /* Character pending on stdin */
    return 1;

  /* no characters were pending */
  return 0;
}

int open_dev_file()
{
	int fd;

	fd = open("/dev/iogen", O_RDONLY);

	if (fd < 0)
	{
		printf("Device file could not be opened. Please ensure the iogen driver module is loaded.\n");
		exit(0);
	}
	
	return fd;

}

void CtrMode(unsigned addr, char cntr, char mode)
{
  int ctrl;
  ctrl = (cntr << 6) | 0x30 | (mode << 1);
  outportb(fd, addr+3, ctrl);
}

void CtrLoad(unsigned addr ,int c,int val)
{
  outportb(fd, addr+c,val & 0x00FF);
  outportb(fd, addr+c,(val>>8) & 0x00FF);
}

unsigned CtrRead(unsigned addr , unsigned int c)
{
	__u8 lo_byte, hi_byte;
	__u16 reading;

	outportb(fd, addr+3,c<<6);

	inportb(fd, addr + c, &lo_byte);

	inportb(fd, addr + c, &hi_byte);

	reading = lo_byte + (hi_byte << 8);

}

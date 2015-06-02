#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>

#include "acceslib.h"


unsigned int baseAddr;
int fd;

unsigned int ask_for_baseAddr(unsigned int old);
int kbhit(void);
int open_dev_file();
unsigned CtrRead(unsigned addr , unsigned int c);
void CtrLoad(unsigned addr ,int c,int val);
void CtrMode(unsigned addr, char cntr, char mode);
void signon(void);
void write_port_data(__u8 current[3]);


int main(void)
{
   __u8    current[3];
   int	        shift_left;

   signon();                          // print the start up message

	fd = open_dev_file();

   outportb(fd, baseAddr + 0x3 ,0x8b);        // send the control byte for A out B in
   current[0] = 0;
   shift_left = 1;
   while(!kbhit())                    // loop until key pressed
      {
      outportb(fd, baseAddr + 0x0,current[0]);     // write value to port a
		usleep(10000); //delay for 0.01 seconds allowing card to settle

		inportb(fd, baseAddr + 0x0, &(current[1])); //read from port A
		inportb(fd, baseAddr + 0x1, &(current[2])); //read from port B


      write_port_data(current);       // write new data to screen

		usleep(700000); //delay for 0.7 seconds so the user doesn't just see 
							//flashing numbers

      // compute value to turn on/off nesxt bit in line
      if (current[0] == 0) shift_left = 1;
      if (current[0] == 255) shift_left = 0;
      if (shift_left) current[0] = (current[0] << 1) + 1;
      else current[0] = (current[0] - 1) >> 1;
      };

		close(fd); //close the device file
} // end main program

void signon(void)
{
   int	index;

   // print the string display and ask for the base address
   puts("                     8255 Digital I/O Sample Program \n"
        "This sample program will sequentially turn on all bits in port a and then\n"
        "seqencially turn them off.  Each time it sets a new bit, both port a and\n"
        "port b are read and the data displayed.  This demonstrates how to read\n"
        "and write to a port, and to use the read back function of the 8255 chip.\n"
        "If the port a pins are jumpered to the port b pins, then a board test\n"
        "program results, with port b being used to verify what has been written\n"
        "to port a.  The program will use port 0 of cards with mulitple 8255's.\n"
       );
   baseAddr = ask_for_baseAddr(0x300);
   puts("\nBoard Configuration:\n");
   printf(" -- Base Address is %X hex\n", baseAddr);
   printf("Connect a loopback cable from PORT A to PORT B of PPI0. (required)\n");
   printf(" PRESS ANY <enter> TO CONTINUE\n");
   getchar ();
} // end signon

void write_port_data(__u8 current[3])
{
   unsigned      x,y, inner, outer;
   unsigned char value;

	system("clear");

	printf("press <enter> to exit\n");

   for (outer = 0; outer <= 2; outer++)       // for each array member
      {
      value = current[outer];

		switch(outer)
		{
			case 0: printf("Written to port A:"); break;
			case 1: printf("Read back from port A:"); break;
			case 2: printf("Read from port B:"); break;
		};

      for (inner = 0; inner <= 7; inner++) // for each bit in array member
         {
         if (value % 2) putchar('1');
         else putchar('0');
         value = value >> 1;                // roll next diaplay bit
         }
		printf("\n");
      }
	fflush(stdout);
}  // end write_port_data

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

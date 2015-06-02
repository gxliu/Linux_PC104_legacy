#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>

#include "acceslib.h"

unsigned int baseAddr;
int fd;

unsigned int ask_for_base(unsigned int old);
int open_dev_file();
int kbhit(void);

int main (void)
{
	int count;
	__u8 reading;
	__u8 output;
	

	fd = open_dev_file();

	baseAddr = ask_for_base(0x300);

	output = 1;

	do
	{
		if (output & 0x80)
		{
			output = 1;
		}
		else
		{
			output <<= 1;
		}

		outportb(fd, baseAddr, output);
		inportb(fd, baseAddr + 1, &reading);

		system("clear");
		
		printf("Press <enter> to exit\n");
		printf("Output to relays: %02X\n", output);
		printf("Reading from inputs: %02X\n", reading);
		
		usleep(500000);

	}while (!kbhit());


	close(fd);


}



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


unsigned int ask_for_base(unsigned int old)
{
	unsigned int newone;
	char buf[1024];
	int count;
	int done = 0;

	do
	{
		printf("Please enter the base address or press enter for %X\n=>", old);

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

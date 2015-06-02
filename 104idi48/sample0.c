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
	__u8 inputs[6];
	int port_addrs[6] = {0, 1, 2, 4, 5, 6}; //the locations where we can read all the ports
	int count;
	
	fd = open_dev_file();

	baseAddr = ask_for_base(0x300);

    do
    {

		system("clear");
		printf("Press <enter> to exit.\n");

        for (count = 0; count < 6; count++)
        {
            inportb(fd, baseAddr + port_addrs[count], &(inputs[count]));
        }

        for (count = 0; count < 6; count++)
        {
            printf("Channel %d : %02X\n", count, inputs[count]);
        }

		usleep(500000); //just to slow things down for our readers

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

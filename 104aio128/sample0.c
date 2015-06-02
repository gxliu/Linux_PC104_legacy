#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>

#include "acceslib.h"



unsigned int ask_for_base(unsigned int old);
int open_dev_file();

int main (void)
{
	unsigned int baseAddr;
	int fd;
	int count;
	__u16 reading;
	__u8 status;

	baseAddr = ask_for_base(0x300);

	fd = open_dev_file();

	printf("This sample will read the values from each of the channels on the board.\n");

	//enable the DACS

	outportb(fd, baseAddr + 0x18, 0x1);

	for(count = 0; count < 8; count ++)
	{
		//tell the board what channel we want to read from
		outportb(fd, baseAddr + 0x02, count);

	
		//poll the board until it says that it is done with the conversion
		do
		{
			inportb(fd, baseAddr, &status);
		}while ((status & 0x80) == 0);

		inport(fd, baseAddr, &reading);

		printf("Reading from channel %d: %02X\n", count, reading);
	}

	close(fd);
		

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

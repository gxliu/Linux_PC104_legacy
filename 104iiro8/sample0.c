#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "acceslib.h"


//function prototypes
unsigned int ask_for_base(unsigned int old);
int open_dev_file ();

int main (void)
{
	int fd;
	int choice;
	int count;
	unsigned int baseAddr;
	__u8 reading;

	baseAddr = ask_for_base(0x300);

	fd = open_dev_file();

	do
	{
		printf("1) Walking bit on Relays.\n");
		printf("2) Read Isolated Inputs.\n");
		printf("3) Read Digital Inputs.\n");
		printf("4) Quit.\n");
		printf("=>");

		scanf("%d", &choice);

		switch(choice)
		{
		case 1:
			for (count = 0; count < 8; count++)
			{
				outportb(fd, baseAddr, 1 << count);
				sleep(1);
			}
			break;

		case 2:
			inportb(fd, baseAddr + 0x1, &reading);
			printf("Reading from Isolated Inputs: %02X\n", reading);
			break;
		case 3:
			inportb(fd, baseAddr + 0x3, &reading);
			printf("Reading from Digital Inputs: %02X\n", reading);
			break;
		};




	}while (choice != 4);

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

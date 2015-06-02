#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>

#include "acceslib.h"

unsigned int baseAddr;
int fd;

unsigned int ask_for_base(unsigned int old);
int open_dev_file();
void write_dac();


int main (void)
{

	char key;

	printf("This sample will allow the user to write a voltage to the DACs\n");
	printf("The requested voltage and actual voltage will be displayed.\n");
	printf("The sample assumes a 0 - 10v range. If your board is set to another range,\n");
	printf("the value will be proportional.\n");

	fd = open_dev_file();

	baseAddr = ask_for_base(0x300);

	//set all the DACs to output 0 volts

	outport(fd, baseAddr + 0x4, 0);
	outport(fd, baseAddr + 0x6, 0);
	outport(fd, baseAddr + 0x8, 0);
	outport(fd, baseAddr + 0xA, 0);


	do
	{
		write_dac();
		printf("\nWould you like to enter another? (Y or N)\n=>");

		do
		{
			key = toupper(getchar());
		} while ((key != 'N') && (key != 'Y'));

		while (getchar() != '\n'); //clear stdin
		
	}	while (key != 'N');

	close(fd);
}


void write_dac()
{
	int dac_num;
	double requested_v, actual_v;
	__u16 counts;

	//get the dac number from user

	do
	{
		printf("\nWhat DAC would you like to output on? (0 - 3)\n=>");
		scanf("%d", &dac_num);
	}while ((dac_num < 0) || (dac_num > 3));

	//get the requested voltage from the user

	do
	{
		printf("\nWhat voltage would you like to output? (0.000 - 9.997)\n=>");
		scanf("%lf", &requested_v);
	}while ((requested_v < 0) || (requested_v > 9.997));


	//calculate the counts for this voltage
	
	counts = (int) (requested_v / 0.002441); //on a 0-10 v range 1 count == 0.00241 volts
	counts &= 0xFFF; //The dac is limited to 12 bits. If the requested_v is valid then
							//this line really shouldn't be neccessary.

	//calculate the actual voltage that the DAC will output

	actual_v = (float) counts * 0.002441;

	//output the voltage to the board

	outport(fd, baseAddr + (dac_num * 2), counts);

	//let the user know what the actual voltage.

	printf("DAC %d is now outputting %4.3f volts.\n", dac_num, actual_v);

	while (getchar() != '\n'); //clear stdin

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

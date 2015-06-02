#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>

#include "acceslib.h"

#define  PI    3.1415927

int fd;
unsigned int baseAddr= 0x300;
int dac;
int points;
unsigned int points_list[20000];

unsigned int ask_for_base(unsigned int old);
int open_dev_file();
int kbhit(void);
void get_params();
void sinecurve();
void sendtoport();
void sawcurve();
void trianglecurve();

int main (void)
{
	int choice;
	
	fd = open_dev_file();

	do
	{
	   printf("\n\n\n");
   	printf("1. Input Board Data  (do this first.)\n");
	   printf("2. Sine curve\n");
   	printf("3. Triangle curve\n");
	   printf("4. Saw curve\n");
   	printf("5. End program, return to DOS\n");
	   printf("=>");

		scanf("%d", &choice);

		switch (choice)
		{
			case 1:
				get_params();
				break;
			case 2:
				sinecurve();
				break;
			case 3:
				trianglecurve();
				break;
			case 4:
				sawcurve();
				break;

		};


	}while (choice != 5);


	close(fd);	
}


void sendtoport()
{
   int           i,temp;
   unsigned char lowbyte,hibyte;

   do
      {
      for( i = 0;i < points;i++)
         {
         outport(fd, baseAddr+0x04+(dac*2), points_list[i]);
         }
      }
   while (!kbhit());
   outport(fd, baseAddr+0x04+(dac*2),0);    // set DAC to 0 output
} // end sendtoport

void sawcurve()
{
   int             i;
   double          slope,temp;

   if (points == 0) return;

   printf("Calculating saw tooth wave points.....\n");

   slope = 4095.0 / points;       /* saw tooth slope */

   for(i = 0;i < points;i++)
      {
      temp = slope * i;
      points_list[i] = (int) temp;
      points_list[i] %= 4096;
      }
   printf("Generating saw tooth wave, press any key to stop....\n");
	fflush(NULL); //be sure all the stuff gets printed before it get's stuck in the loop
   sendtoport();
} // end sawcurve

void trianglecurve()
{
   int             i;
   double          slope,temp;

   if (points == 0) return;            // no counts -- no curve

   printf("Calculating triangle wave points.....\n");

   slope = 4095.0 / points * 2.0;
// wave form slope
   for(i=0;i < points/2;i++)
      {
      temp = slope * i;
      points_list[i] = (int)temp;
      temp = 4095 - temp;
      points_list[i+points/2+1] = (int)temp;
      }
   printf("Generating triangle wave, press any key to stop....\n");
	fflush(NULL); //be sure all the stuff gets printed before it get's stuck in the loop
   sendtoport();
}  // end trianglecurve

void get_params()
{
	getchar(); //clear the left over character on stdin

	baseAddr = ask_for_base(baseAddr);

	dac = 5;

	do
	{
		printf("\nWhich dac?(0 to 3)\n=>");
		scanf("%d", &dac);
	}while ((dac < 0) || (dac > 3));

	do
	{
		printf("\nHow many points? (10 to 20000)\n=>");
		scanf("%d", &points);
	}while ((points < 10) || (points > 20000));

	

}

void sinecurve()
{
   int             i;
   double          rads,sine;

   if (points == 0) return;           // no point -- no curve

	
   printf("Calculating sine wave points.....\n");

   rads = (double) 2 * PI / (points - 1);   // rad per count

   for(i = 0;i < points;i++)
      {
      sine = (sin(rads * i) + 1.0) * 2047;
      points_list[i] = (unsigned) sine;
      }
   printf("Generating sine wave, press any key to stop....\n");
	fflush(NULL); //be sure all the stuff gets printed before it get's stuck in the loop
   sendtoport();
} // end sinecurve

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

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>

#include "acceslib.h"

//All these defines are part of the 104-QUAD-8 specific functions

#define RLD 0x80
#define XRLD 0x00
#define YRLD XRLD
#define Rst_BP 0x01
#define Rst_CTR 0x02
#define Rst_FLAGS 0x04
#define Rst_E 0x06
#define Trf_PR_CTR 0x08
#define Trf_CTR_OL 0x10
#define Trf_PS0_PSC 0x18

#define CMR 0xA0
#define XCMR 0x20
#define YCMR XCMR
#define BINCnt 0x00
#define BCDCnt 0x01
#define NrmCnt 0x00
#define RngLmt 0x02
#define NRcyc 0x04
#define ModN 0x06
#define NQDX 0x00
#define QDX1 0x08
#define QDX2 0x10
#define QDX4 0x18

#define IOR 0xC0
#define XIOR 0x40
#define YIOR 0x00
#define DisAB 0x00
#define EnAB 0x01
#define LCTR 0x00
#define LOL 0x02
#define RCTR 0x00
#define ABGate 0x04
#define CYBW 0x00
#define CPBW 0x08
#define CB_UPDN 0x10
#define IDX_ERR 0x18

#define IDR 0xE0
#define XIDR 0x60
#define YIDR XIDR
#define DisIDX 0x00
#define EnIDX 0x01
#define NIDX 0x00
#define PIDX 0x02
#define LIDX 0x00
#define RIDX 0x04

//104-QUAD-8 specific functions:
void Init_7266(unsigned long Addr);
void Write_7266_PR(unsigned long Addr, unsigned long Data);
long int Read_7266_OL(unsigned long Addr);

unsigned int baseAddr;
int fd;

unsigned int ask_for_base(unsigned int old);
int open_dev_file();
int kbhit(void);
float getTicks(void);

int main (void)
{
    int done = 0;
    float ticksPerInch;
    long int reading;
	 int count;

    baseAddr = ask_for_base(0x300);

    Init_7266(baseAddr); //does all counters wierdly


    ticksPerInch = getTicks();


    while (!kbhit())
    {
		system("clear");

       printf("\nNow reading all channels.\nPress <enter> key to exit...\n");

       for (count = 0; count < 8; count++)
       {

          reading = Read_7266_OL(baseAddr + count * 2);

          printf("Channel %d: Reading:%08lx(hex) Distance:%.2f (inches) \n", count, reading,
                  reading/ticksPerInch );
       }




    };

}

float getTicks(void)
{
   int done = 0;
   char line[256];
   float result;

   while (!done)
   {

      printf("Enter the number of ticks per inch: ");
      gets(line);

      if (sscanf(line, "%f", &result) == 1)
         done = 1;

   };

   return result;


}

long int Read_7266_OL(unsigned long Addr)
{
   union pos_tag {     /* allows access of 32-bit integer as 4 bytes */
   long int l;
   struct byte_tag {char b0; char b1; char b2; char b3;} byte;
   }pos;

   outportb(fd, Addr + 1, 0x10);   /* reset address pointer */
   outportb(fd, Addr + 1, 0x01);      /* command to latch counter */

	inportb(fd, Addr, (__u8*)(&(pos.byte.b0)));
	inportb(fd, Addr, (__u8*)(&(pos.byte.b1)));
	inportb(fd, Addr, (__u8*)(&(pos.byte.b2)));


   /* extend sign of position */  //requires signed characters
   if (pos.byte.b2 < 0) pos.byte.b3 = -1;
   else pos.byte.b3 = 0;

   return pos.l;
}

void Write_7266_PR(unsigned long Addr, unsigned long Data)
{

   outportb(fd, Addr + 1, RLD + Rst_BP);
   outportb(fd, Addr + 0, Data);
   Data >>= 8;
   outportb(fd, Addr + 0, Data);
   Data >>= 8;
   outportb(fd, Addr + 0, Data);
}

void Init_7266(unsigned long Addr)
{
	int count;

   outportb(fd, Addr+0x14,0x01); //put CPLD on card into divide-by-1 FCK mode

    for (count = 0; count < 4; count++)
    {
      unsigned long XCt = 0xFFFFFF;
      unsigned long YCt = 0xFFFFFF;

       //Setup IOR reg
       outportb(fd, Addr + 1, IOR + DisAB + LOL + ABGate + CYBW);

       //Setup RLD reg
       outportb(fd, Addr + 1, RLD + Rst_BP + Rst_FLAGS);
       outportb(fd, Addr + 0, 0x00);
       outportb(fd, Addr + 2, 0x00);
       outportb(fd, Addr + 1, RLD + Rst_E + Trf_PS0_PSC);

       //Setup IDR reg
       outportb(fd, Addr + 1, IDR + EnIDX + NIDX + LIDX);

       //Setup CMR reg
       outportb(fd, Addr + 1, CMR + BINCnt + ModN + QDX1);

       //Setup PR reg for modulo N counter to XCt
       outportb(fd, Addr + 0, XCt);
       XCt >>= 8;
       outportb(fd, Addr + 0, XCt);
       XCt >>= 8;
       outportb(fd, Addr + 0, XCt);

       //Setup PR reg for modulo N counter to YCt
       outportb(fd, Addr + 2, YCt);
       YCt >>= 8;
       outportb(fd, Addr + 2, YCt);
       YCt >>= 8;
       outportb(fd, Addr + 2, YCt);

       //Enable counters
       outportb(fd, Addr + 1, IOR + EnAB);
       Addr += 4;
    }
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

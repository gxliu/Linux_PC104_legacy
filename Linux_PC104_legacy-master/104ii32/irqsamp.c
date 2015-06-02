///////////////////////////////////////////////////////////
//This sample demonstrates how to use the irq ioctl with 
//the ACCES 104-II32-4RO using fork().
//if you understand fork() then this will probably seem a very
//simplistic sample. If you don't then it is recommended you
//familarize yourself with fork(), pipe(), and wait() even though
//fork() is the only one used in this sample.
///////////////////////////////////////////////////////////


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "acceslib.h"
#include "iogen.h"

unsigned int baseAddr;
int fd;

unsigned int ask_for_base(unsigned int old);
int open_dev_file();
void post_fork_child();
void post_fork_parent();
int kbhit(void);

int main (void)
{
	int status;

	fd = open_dev_file();

	baseAddr = ask_for_base(0x300);

	status = fork();

	printf("Status from fork: %d\n", status);

	if (status > 0) //we are the parent
	{
		printf("Child ps id: %d\n", status);
		fflush(stdout);
		post_fork_parent();
	}
	else if (status == 0) //we are the child
	{
		post_fork_child();
	}
	else //fork() failed. not good
	{
		printf("Fork failed.\n");
	}


	fflush(stdout);

	close(fd);
}

void post_fork_child()
{

	sleep(1); //using a pipe or a signal would be a better way of making sure
				//the parent process finishes its setup, but for now we will just
				//give it more than enough time.

	printf("Waiting for IRQ. Press <enter> to exit\n");

	while (!kbhit());

	printf("Child sending cancel\n");

	fflush(stdout);

	ioctl(fd, io_gen_cancel_wait_for_irq_ioctl);

}
void post_fork_parent()
{
	io_gen_irq_struct irq_request;
	int status;
	__u8 readings[4];
	int count;
	char line[1024];


	irq_request.offset_to_clear = baseAddr + 0x05;
	irq_request.irq_num = 5;
	irq_request.write_to_clear = 1; //yes, it is write to clear
	irq_request.val_to_clear = 0; //any value will work with this card
	

   status = ioctl(fd, io_gen_irq_setup, &irq_request);

	if (status != 0)
	{
		printf("Failed to allocate IRQ. Parent exiting.\n");
		return; //don't want to actually exit. give it a chance to close(fd) properly in main
	}

	//now that we have the IRQ we need to tell the card to generate them on all ports

	outportb(fd, baseAddr + 0x04, 0xFF);
	usleep(100000);

	//clear any random interrupt on the board
	outportb(fd, baseAddr + 0x05, 0x00);

	do
	{
		status = ioctl(fd, io_gen_wait_for_irq_ioctl);

		if (status == 0)
		{
			for (count = 0; count < 4; count++)
			{
				inportb(fd, baseAddr + count, &(readings[count]));
			}

			sprintf(line, "Interrupt occurred. Press <enter> to exit\nPort 0: %02X\nPort 1: %02X\nPort 2: %02X\nPort 3: %02X\n",
						readings[0], readings[1], readings[2], readings[3]);

			printf(line);
		}
		else
		{
			perror("The following error occurred while waiting for irq");
		}
			

	}while(status == 0);

	//let the board know not to generate any more interrupts
	outportb(fd, baseAddr + 0x04, 0x00);

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


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

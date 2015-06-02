#include <sys/ioctl.h>
#include <linux/errno.h>

#include "acceslib.h"
#include "iogen.h"



int outportb (int fd, unsigned int offset, __u8 data)
{
		io_gen_io_packet ioPack;

		ioPack.size = 0;
		ioPack.offset = offset;
		ioPack.data = data;

		return ioctl(fd, io_gen_write_ioctl, &ioPack);
}

int outport(int fd, unsigned int offset, __u16 data)
{
	io_gen_io_packet ioPack;
	
	ioPack.size = 1;
	ioPack.offset = offset;
	ioPack.data = data;

	return ioctl(fd, io_gen_write_ioctl, &ioPack);
}

int outportl (int fd, unsigned int offset, __u32 data)
{
	io_gen_io_packet ioPack;

	ioPack.size = 2;
	ioPack.offset = offset;
	ioPack.data = data;

	return ioctl(fd, io_gen_write_ioctl, &ioPack);
}

int inportb(int fd, unsigned int offset, __u8 *data)
{
	io_gen_io_packet ioPack;
	int status;

	ioPack.size = 0;
	ioPack.offset = offset;
	
	status = ioctl(fd, io_gen_read_ioctl, &ioPack);

	*data = ioPack.data;

	return status;

}

int inport(int fd, unsigned int offset, __u16 *data)
{
	io_gen_io_packet ioPack;
	int status;

	ioPack.size = 1;
	ioPack.offset = offset;

	status = ioctl(fd, io_gen_read_ioctl, &ioPack);

	*data = ioPack.data;
	
	return status;
}

int inportl(int fd, unsigned int offset, __u32 *data)
{
	io_gen_io_packet ioPack;
	int status;

	ioPack.size = 2;
	ioPack.offset = offset;

	status = ioctl(fd, io_gen_read_ioctl, &ioPack);
}

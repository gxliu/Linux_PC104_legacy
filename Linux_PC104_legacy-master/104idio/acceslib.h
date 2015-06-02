#include <asm/types.h>

int outportb (int fd, unsigned int offset, __u8 data);
int outport (int fd, unsigned int offset, __u16 data);
int outportl (int fd, unsigned int offset, __u32 data);

int inportb(int fd, unsigned int offset, __u8 *data);
int inport(int fd, unsigned int offset, __u16 *data);
int inportl (int fd, unsigned int offset, __u32 *data);


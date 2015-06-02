#include <asm/types.h>

__u8 outportb (int fd, unsigned int offset, __u8 data);
__u16 outport (int fd, unsigned int offset, __u16 data);
__u32 outportl (int fd, unsigned int offset, __u32 data);

__u8 inportb(int fd, unsigned int offset, __u8 *data);
__u16 inport(int fd, unsigned int offset, __u16 *data);
__u32 inportl (int fd, unsigned int offset, __u32 *data);


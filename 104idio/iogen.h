//for the magic number I have decided to use 0xE0. There really is no
//reason for this choice other than the fact that it is not listed as taken in
//ioctl-number.txt and it is pretty obvious that no  one is updating the list anyway.
//to make it easier to change in the future I will define it

#define ACCES_MAGIC_NUM 0xE0
#define MAX_REGIONS		16


//Uncomment the following line to enable debug messages
#define IO_GEN_DEBUG_ON

typedef struct
{
	int size; //enumerated value 0 = byte, 1 = word, 2 = DWORD;
	unsigned int offset; //the offset 
	unsigned long data; //the data to be written. Will be truncated depending on length.
}io_gen_io_packet;

typedef struct
{
	unsigned int start; //the start of the region being requested
	unsigned int end; //the last address being requested
}io_gen_resource;

typedef struct
{
	unsigned int offset_to_clear;
	int irq_num;
	int write_to_clear; //1 if write to clear, 0 if read to clear
	unsigned char val_to_clear; //if it is write to clear than this is the value to write
}io_gen_irq_struct;

#define io_gen_write_ioctl _IOW(ACCES_MAGIC_NUM, 1, io_gen_io_packet *)
#define io_gen_read_ioctl _IOR(ACCES_MAGIC_NUM, 2, io_gen_io_packet *)
#define io_gen_wait_for_irq_ioctl _IO(ACCES_MAGIC_NUM, 3)
#define io_gen_cancel_wait_for_irq_ioctl _IO(ACCES_MAGIC_NUM, 4)
#define io_gen_request_io _IO(ACCES_MAGIC_NUM, 5)
#define io_gen_irq_setup _IOW(ACCES_MAGIC_NUM, 6, io_gen_irq_struct *)


#define IO_GEN_MAXNR 6

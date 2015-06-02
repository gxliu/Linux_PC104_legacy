#include <linux/module.h>
#include <linux/proc_fs.h>

#include <linux/ioport.h> //request_region
#include <linux/cdev.h> //cdev struct and cdev_add etc
#include <asm/uaccess.h> //copy_from/to_user
#include <asm/io.h>

#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/fs.h>

#include "iogen.h"


MODULE_LICENSE("GPL");


typedef struct
{
	unsigned int start;
	unsigned int end;
	struct file *filp;
}io_resource_descriptor;

io_resource_descriptor io_descriptor_array[MAX_REGIONS];
int current_regions = 0;


//global variables
static int major_num = 0; //our major number

//int base_addr = 0x300;	//the base address of the card

// init the IRQ relevant variables to illegal values so we can be sure that
// we are ready when a wait for IRQ requested

int write_to_clear = -1;	//0 if the card is read from to clear
int offset_to_clear = -1;	//the offset from the base address where the register to clear is
int value_to_clear = 0x00;	//the value to be written on a write to clear board
int irq_num = -1;	//the IRQ assigned to the card
//int space = 0x4; //how much address space does the card need?
static int waiting_for_irq = 0; //boolean for if we have been asked to process IRQs. If not then ignore.
static int irq_cancelled = 0;

DECLARE_WAIT_QUEUE_HEAD(io_gen_wait);

struct resource *presource;

//module parameters

//module_param(base_addr, int, 0);
//module_param(write_to_clear, int, 0);
//module_param(offset_to_clear, int, 0);
//module_param(value_to_clear, int, 0);
//module_param(irq_num, int, 0);
//module_param(space, int, 0);


//irqreturn_t iogen_interrupt(int irq, void *dev_id, struct pt_regs *regs);

//fops function declarations
int io_gen_open (struct inode *inode, struct file *filp);
int io_gen_release (struct inode *inode, struct file *filp);
ssize_t io_gen_read (struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t io_gen_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos);

#ifdef HAVE_UNLOCKED_IOCTL 
long io_gen_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else 
int io_gen_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif


int get_irq (int is_write, int offset, int irq_requested, unsigned char to_clear);


//fops structure
struct file_operations io_gen_fops = {
#ifdef HAVE_UNLOCKED_IOCTL 
    .unlocked_ioctl = io_gen_ioctl,
#else
    .ioctl = io_gen_ioctl,
#endif
    .read = io_gen_read,
    .write = io_gen_write,
    .open = io_gen_open,
    .release = io_gen_release,
};

//cdev stuff
struct cdev io_gen_cdev;


ssize_t io_gen_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return count; //for now we will pretend we read the data so it will kind of be like
						//writing to /dev/NULL I _think_. Can't just return 0 as the kernel
						//will just keeping retrying the operation
}


ssize_t io_gen_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0; //I think this will just make any read from the device look like End Of File
					//which is not such a bad thing
}

int make_region_request(io_gen_resource resource_requested, struct file *filp)
{
	int count;

	//check if we have room
	if (current_regions == MAX_REGIONS) return -1;

	//check to make sure the range is valid

	if (((resource_requested.start > resource_requested.end) ||
		(resource_requested.start < 100) || (resource_requested.start > 0x3ff)) ||
		((resource_requested.end < 100) || (resource_requested.end > 0x3ff)))
	{
		return -EDOM;
	}
		

	//check to see if we already have this resource

	for (count = 0; count < current_regions; count++)
	{
		if ((resource_requested.start >= io_descriptor_array[count].start &&
				resource_requested.start <= io_descriptor_array[count].end) ||
			(resource_requested.end <= io_descriptor_array[count].end &&
				resource_requested.end >= io_descriptor_array[count].end))
		{
			return -EFAULT;
		}
	}
	

	presource = request_region((unsigned long) resource_requested.start,
										 (unsigned long) resource_requested.end - resource_requested.start + 1,
										 "iogen");

	if (presource != NULL)
	{
		io_descriptor_array[current_regions].start = resource_requested.start;
		io_descriptor_array[current_regions].end = resource_requested.end;
		io_descriptor_array[current_regions].filp = filp;
		current_regions++;
		return 0;
	}
	else
	{
		return -EACCES;
	}
}

#ifdef HAVE_UNLOCKED_IOCTL 
long  io_gen_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else 
int io_gen_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	io_gen_io_packet iopack;
	io_gen_resource resource_requested;
	io_gen_irq_struct irq_request;
	int status;

	switch (cmd)
	{
		case io_gen_request_io:
			if (access_ok(VERIFY_WRITE, arg, sizeof(io_gen_resource)))
			{
				status = copy_from_user(&resource_requested, (io_gen_resource *)arg, sizeof(io_gen_resource));

				if (status != 0) return status;

				return make_region_request(resource_requested, filp);
			}
			else
			{
#ifdef IO_GEN_DEBUG_ON
				printk(KERN_INFO "iogen: could not copy from user\n");
#endif
				return -EFAULT;
			}
				
		case io_gen_write_ioctl:
			if (access_ok(VERIFY_WRITE, arg, sizeof(io_gen_io_packet)))
			{
				//grab the data and write it to the offset....
				status = copy_from_user(&iopack, (io_gen_io_packet *)arg, sizeof(io_gen_io_packet));
				
				if (status != 0) return status;

#ifdef IO_GEN_DEBUG_ON
				printk(KERN_INFO "iogen: attempting to write to address %x\n", iopack.offset);
#endif

				//since the request region doesn't seem to be enforced we are going
				//to disallow writes and reads below 100 hex

				if (iopack.offset <= 0x100)
				{
					 return -EFAULT;
				}

				switch (iopack.size)
				{
					case 0:
						outb((unsigned char) iopack.data, iopack.offset);
						break;

					case 1:
						outw((unsigned short) iopack.data,iopack.offset);
						break;
					case 2:
						outl((unsigned long) iopack.data, iopack.offset);
						break;
				};
				return 0;
			}
			else //couldn't acces the data. oh nos!
			{
#ifdef IO_GEN_DEBUG_ON
				printk(KERN_INFO "iogen: could not get access io_gen_write_ioctl\n");
#endif
				return -EFAULT;
			}
		break;
		case io_gen_read_ioctl:
			if (access_ok(VERIFY_WRITE, arg, sizeof(io_gen_io_packet)))
			{
				status = copy_from_user(&iopack, (io_gen_io_packet *)arg, sizeof(io_gen_io_packet));

				if (status != 0) return status;
#ifdef IO_GEN_DEBUG_ON
				printk(KERN_INFO "iogen: attempting to read from address %x\n", iopack.offset);
#endif

				if (iopack.offset <= 0x100)
				{
					return -EFAULT;
				}

				switch(iopack.size)
				{
					case 0:
						iopack.data = inb(iopack.offset);
						break;

					case 1:
						iopack.data = inw(iopack.offset);
						break;

					case 2:
						iopack.data = inl(iopack.offset);
						break;
				};

				return copy_to_user((io_gen_io_packet *)arg, &iopack, sizeof(io_gen_io_packet));

			}
			else
			{
#ifdef IO_GEN_DEBUG_ON
				printk(KERN_INFO "iogen: could not get access: io_gen_read_ioctl\n");
#endif
				return -EFAULT;
			}
		case io_gen_wait_for_irq_ioctl:
			if (waiting_for_irq != 0)
			{
#ifdef IO_GEN_DEBUG_ON
				printk(KERN_INFO "iogen: io_gen_wait_for_irq_ioctl called while already waiting\n");
#endif
				return -EBUSY;
			}
			else
			{

				if ((irq_num == -1) || (offset_to_clear == -1) || (write_to_clear == -1))
				{
#ifdef IO_GEN_DEBUG_ON
					printk(KERN_INFO "iogen: attempt to wait for IRQ without allocation\n");
#endif
					//we aren't ready because the irq has not been properly requested/allocated yet
					return -EFAULT;
				}

				waiting_for_irq = 1;
#ifdef IO_GEN_DEBUG_ON
				printk(KERN_INFO "iogen: io_gen_wait_for_irq_ioctl: entering sleep\n");
#endif
				interruptible_sleep_on(&io_gen_wait);
#ifdef IO_GEN_DEBUG_ON
				printk(KERN_INFO "iogen: io_gen_wait_for_irq_ioctl: done sleeping\n");
#endif

				if (irq_cancelled == 1)
				{
					irq_cancelled = 0;
					return -ECANCELED;
				}
				else
				{
					return 0;
				}
			}
			break;
		case io_gen_irq_setup:
			if (irq_num != -1) //we already have an irq
			{
				return -EBUSY;
			}
			if (access_ok(VERIFY_WRITE, arg, sizeof(io_gen_irq_struct)))
			{
				status = copy_from_user(&irq_request, (io_gen_irq_struct *)arg, sizeof(io_gen_irq_struct));

				return get_irq(irq_request.write_to_clear, 
									irq_request.offset_to_clear, 
									irq_request.irq_num, 
									irq_request.val_to_clear);

			}
			else
			{
				return -EFAULT;
			}
			break;
		case io_gen_cancel_wait_for_irq_ioctl:
#ifdef IO_GEN_DEBUG_ON
			printk(KERN_INFO "iogen: cancelling irq wait\n");
#endif
				waiting_for_irq = 0; 
				irq_cancelled = 1;
				wake_up_interruptible(&io_gen_wait);
//				while (irq_cancelled == 1); //wait for the other ioctl to finish then return.
				return 0;
				break;
		default:
			return -EINVAL; //probably need a better return val
	};
}


irqreturn_t iogen_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned char toss;
#ifdef IO_GEN_DEBUG_ON
	printk (KERN_INFO "iogen: irq handler called waiting=%d\n", waiting_for_irq);
#endif

	if (write_to_clear)
	{
		outb((unsigned char) value_to_clear, offset_to_clear);
	}
	else //I don't think any of our cards make it worthwhile to remember the value read
	{		//but I will look into it and possibly set up a method for that later
		toss = inb(offset_to_clear);
	}

	if (waiting_for_irq)
	{
		waiting_for_irq = 0;
		wake_up_interruptible(&io_gen_wait);
	}

	return IRQ_HANDLED;

}

int get_irq (int is_write, int offset, int irq_requested, unsigned char to_clear)
{
	int result;

	//first make sure our parameters are all good

	if (!((is_write == 0) || (is_write == 1)))
	{
#ifdef IO_GEN_DEBUG_ON
		printk(KERN_INFO "iogen: get_irq called with invalid write_to_clear\n");
#endif
		return -EDOM;
	}

	if ((offset < 0x100) || (offset > 0x3FF))
	{
#ifdef IO_GEN_DEBUG_ON
		printk(KERN_INFO "iogen: get_irq called with invalid offset\n");
#endif
		return -EDOM;
	}

	if ((irq_requested < 2) || (irq_requested > 15))
	{
#ifdef IO_GEN_DEBUG_ON
		printk(KERN_INFO "iogen: get_irq called with invalid irq_num\n");
#endif
		return -EDOM;
	}

	result = request_irq((unsigned int)irq_requested, iogen_interrupt, 0, "iogen", NULL);

	if (result == 0)
	{
#ifdef IO_GEN_DEBUG_ON
		printk(KERN_INFO "iogen: irq granted.\n");
#endif
		write_to_clear = is_write;
		offset_to_clear = offset;
		irq_num = irq_requested;
		value_to_clear = to_clear;
	}

	return result;

}

int io_gen_open (struct inode *inode, struct file *filp)
{
	int result = 0;
#ifdef IO_GEN_DEBUG_ON
	printk(KERN_INFO "iogen: entering io_gen_open\n");
#endif
/*
	presource = request_region((unsigned long) 100, (unsigned long) 0x3ff, "iogen");

	if (!presource) //we didn't get the pointer
		result = -1; //need to figure out error code to assign
	else
	{
		//now to request the IRQ
		//passing NULL for the last parameter is only OK because we are not sharing the Interrupt
#ifdef IO_GEN_DEBUG_ON
		printk(KERN_INFO "iogen: I/O region access granted\n");
#endif
		result = request_irq((unsigned int)irq_num, iogen_interrupt, 0, "iogen", NULL);

		if (result == 0)
		{
#ifdef IO_GEN_DEBUG_ON
			printk(KERN_INFO "iogen: IRQ granted\n");
#endif
			enable_irq((unsigned int)irq_num);
		}
		else
		{
#ifdef IO_GEN_DEBUG_ON
			printk(KERN_INFO "iogen: IRQ not granted");
#endif
		}

	}	
*/
	return result;
	
}

int io_gen_release (struct inode *inode, struct file *filp)
{

	int count, inner;

	if (current_regions)
	{
		count = 0;

		do
		{
			if (io_descriptor_array[count].filp == filp)
			{
				release_region((unsigned long) io_descriptor_array[count].start,
									(unsigned long) io_descriptor_array[count].end - io_descriptor_array[count].start + 1);

				if (count == current_regions - 1)
				{
					current_regions--;
				}
				else
				{
					for (inner = count; inner < current_regions; inner++)
					{
						io_descriptor_array[count].start = io_descriptor_array[count + 1].start;
						io_descriptor_array[count].end = io_descriptor_array[count + 1].end;
						io_descriptor_array[count].filp = io_descriptor_array[count + 1].filp;
					}
					current_regions--;
				}
			}
			else
			{
				count++;
			}


		}while (count < current_regions);
		

	}
	

	free_irq(irq_num, NULL);


//	waiting_for_irq = 0;
	
//	disable_irq((unsigned int)irq_num);

#ifdef IO_GEN_DEBUG_ON
	printk(KERN_INFO "iogen: finished io_gen_release\n");
#endif

	return 0;

}

static int io_gen_init(void)
{
	//get our major device number and for now only request 1 minor number
	int result;

	dev_t dev = MKDEV(0, 0); //set it to 0 for error check after alloc
#ifdef IO_GEN_DEBUG_ON
	printk(KERN_INFO "iogen: entering io_gen_INIT\n");
#endif

	result = alloc_chrdev_region(&dev, 0, 1, "iogen\n");

	major_num = MAJOR(dev);

	if (!major_num)
		return result; //return the error code if we didn't get a major number
	else
	{
#ifdef IO_GEN_DEBUG_ON
		printk(KERN_INFO "iogen: major num granted\n");
#endif
	}

	cdev_init(&io_gen_cdev, &io_gen_fops);

	io_gen_cdev.owner = THIS_MODULE;
	io_gen_cdev.ops = &io_gen_fops;
	

	result = cdev_add(&io_gen_cdev, dev, 1);

	if (result < 0)
	{
#ifdef IO_GEN_DEBUG_ON
		printk(KERN_INFO "iogen: cdev_add failed in io_gen_init\n");
#endif
		return result;
	}

	return 0;
}

static void io_gen_exit(void)
{
	//need to release our major number
#ifdef IO_GEN_DEBUG_ON
	printk(KERN_INFO"iogen: entering io_gen_EXIT\n");
#endif

	unregister_chrdev_region(MKDEV(major_num, 0), 1);

//	unregister_chrdev_region (dev, 1);
	cdev_del(&io_gen_cdev);

	//this function can't fail and no return value is expected
	
}



module_init(io_gen_init);
module_exit(io_gen_exit);

















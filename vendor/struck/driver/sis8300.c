/*
 * SIS8300 Dev Driver
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/version.h>

#include "sis8300.h"
#include "sis8300_defs.h"
#include "sis8300_reg.h"
#include "sis8300_ioctl.h"
#include "sis8300_read.h"
#include "sis8300_write.h"
#include "sis8300_irq.h"

sis8300_dev *gdevice;

sis8300_dev *sis8300_devdata[SIS8300_MAXCARDS];

MODULE_AUTHOR("SIS GmbH <info@struck.de>");
MODULE_DESCRIPTION("SIS8300/SIS8300L/SIS8325 Digitizer, SIS8800 Counter");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_SUPPORTED_DEVICE("sis8300, sis8325, sis8800");

/***************************************************************/

/*
 * PCI Device Struct
 */
const struct pci_device_id sis8300_id[] = {
	{ PCI_DEVICE(PCI_VENDOR_FZJZEL, PCI_PRODUCT_SIS8300) },
	{ PCI_DEVICE(PCI_VENDOR_FZJZEL, PCI_PRODUCT_SIS8300L) },
	{ PCI_DEVICE(PCI_VENDOR_FZJZEL, PCI_PRODUCT_SIS8325) },
	{ PCI_DEVICE(PCI_VENDOR_FZJZEL, PCI_PRODUCT_SIS8800) },
	{ 0 }
};

MODULE_DEVICE_TABLE(pci, sis8300_id);

/***************************************************************/

/*
 * File Operations
 */

static struct file_operations sis8300_fops = {
	.owner = THIS_MODULE,
	.open = sis8300_open,
	.read = sis8300_read,
	.write = sis8300_write,
	.unlocked_ioctl = sis8300_ioctl,
	.release = sis8300_release,
};

/***************************************************************/

int sis8300_open(struct inode *inode, struct file *file){

	/*
	 * fill in the device struct into the file struct so
	 * ioctl, read, write, release
	 * can use the stuff (bar0, etc) that gets passed to them
	 */

	int i;
	
	for(i = 0;i < SIS8300_MAXCARDS;i++){
	  if(MAJOR(sis8300_devdata[i]->drvnum) == imajor(inode)){
	    file->private_data = (sis8300_dev *)sis8300_devdata[i];
	    break;
	  }
	}
	return 0;
}

/***************************************************************/

int sis8300_release(struct inode *inode, struct file *file){

	file->private_data = NULL;

	return 0;
}

/***************************************************************/

//static int __devinit init_sis8300(struct pci_dev *pdev, const struct pci_device_id *ent)
static int init_sis8300(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int res;
	uint32_t bar0_res, bar0_size;
	uint16_t linkstatus, linkspeed;
	int cardNum;
	char cardName[16];
	static int cardTypeCount[SIS8xxx_CARD_TYPES] = {0};

	sis8300_dev *sisdevice;
 
	/*
	 * allocate zero'ed kernel memory for the device struct
	 */

	// check if there were 4 cards already registered
	for(cardNum = 0;cardNum < SIS8300_MAXCARDS;cardNum++){
	  if(!sis8300_devdata[cardNum])
	    break;
	}
	
	if(cardNum == SIS8300_MAXCARDS){
	  printk(KERN_INFO "sis8300: driver only supports %d cards.\n", SIS8300_MAXCARDS);
	  goto err_endevice;
	}
	
	sisdevice = kzalloc(sizeof(sis8300_dev), GFP_KERNEL);
	if(!sisdevice){
		return -ENOMEM;
	}

	/* build device node name based on card id and number of each card */
	sis8300_devdata[cardNum] = sisdevice;
	switch (pdev->device) {
	case PCI_PRODUCT_SIS8300:
	case PCI_PRODUCT_SIS8300L:
		sprintf(cardName, "sis8300-%d", cardTypeCount[SIS8300]++);
		break;
	case PCI_PRODUCT_SIS8325:
		sprintf(cardName, "sis8325-%d", cardTypeCount[SIS8325]++);
		break;
	case PCI_PRODUCT_SIS8800:
		sprintf(cardName, "sis8800-%d", cardTypeCount[SIS8800]++);
		break;
	}

	res = pci_enable_device(pdev);
	if(res){
		printk(KERN_ALERT "%s: can't enable pci device: %d\n", cardName, res);
		goto err_endevice;
	}
	
	pci_read_config_word(pdev, 0x72, &linkstatus);
	linkspeed = linkstatus & 0xF;
	linkstatus = (linkstatus & 0x1F0) >> 4;
	printk(KERN_INFO "%s: PCIExpress link status: %i lane%sat 2.5Gb/s\n",
		cardName,
		linkstatus,
		linkstatus > 1 ? "s " : " ");
	printk(KERN_INFO "%s: PCIExpress link speed: %i\n",
		cardName,
		linkspeed);
	
	printk(KERN_INFO "%s: vendor id: %04X/device id: %04X\n", cardName, pdev->vendor, pdev->device);

	res = pci_request_regions(pdev, "sis8300");
	if(res){
		printk(KERN_ALERT "%s: can't request pci regions: %d\n", cardName, res);
		goto err_reqregions;
	}

	sisdevice->bar0 = pci_iomap(pdev, 0, 0);
	if(!(sisdevice->bar0)){
		printk(KERN_ALERT "%s: can't get bar0 address\n", cardName);
		goto err_iomap;
	}
	
	bar0_size = pci_resource_end(pdev, 0);
	bar0_res = pci_resource_start(pdev, 0);
	bar0_size -= bar0_res;

	printk(KERN_INFO "%s: bar0 mapped to:%p size:%d\n", cardName, sisdevice->bar0, bar0_size + 1);

	printk(KERN_INFO "%s: dma mask set\n", cardName);

	sisdevice->drvclass = class_create(THIS_MODULE, cardName);
	if(IS_ERR(sisdevice->drvclass)){
		printk(KERN_ALERT "%s: can't create class\n", cardName);
		goto err_class;
	}

	printk(KERN_INFO "%s: class created\n", cardName);

	res = alloc_chrdev_region(&(sisdevice->drvnum), 0, 1, cardName);
	if(res){
		printk(KERN_ALERT "%s: can't alloc chardev: %d\n", cardName, res);
		goto err_chrdev;
	}

	printk(KERN_INFO "%s: char device allocated\n", cardName);

	sisdevice->drvcdev = cdev_alloc();

	cdev_init(sisdevice->drvcdev, &sis8300_fops);

	res = cdev_add(sisdevice->drvcdev, sisdevice->drvnum, 1);
	if(res){
		printk(KERN_ALERT "%s: can't add cdev: %d\n", cardName, res);
		goto err_cdevadd;
	}

	printk(KERN_INFO "%s: char device added\n", cardName);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	sisdevice->drvdevice = device_create(sisdevice->drvclass, &pdev->dev, sisdevice->drvnum, NULL, cardName);
#else
	sisdevice->drvdevice = device_create(sisdevice->drvclass, &pdev->dev, sisdevice->drvnum, cardName);
#endif
	if(IS_ERR(sisdevice->drvdevice)){
		printk(KERN_ALERT "%s: can't create device\n", cardName);
		goto err_devcreate;
	}

	printk(KERN_INFO "%s: device created\n", cardName);
	
	printk(KERN_INFO "%s: device number: major:%d minor:%d\n", cardName, MAJOR(sisdevice->drvnum), MINOR(sisdevice->drvnum));

	pci_set_drvdata(pdev, sisdevice);

	sisdevice->pdev = pdev;

	sisdevice->dmablock.data = (uint8_t *)pci_alloc_consistent(pdev,
							SIS8300_KERNEL_DMA_BLOCK_SIZE,
							&sisdevice->dmablock.dma_addr);
	if(!(sisdevice->dmablock.data)){
	  printk(KERN_INFO "%s: can't allocate dma kernel memory\n", cardName);
	  goto err_devcreate;
	}
	sisdevice->dmablock.size = SIS8300_KERNEL_DMA_BLOCK_SIZE;
	
	printk(KERN_INFO "%s: dma sw buffer:%p len:%Zu\n", cardName, sisdevice->dmablock.data, sisdevice->dmablock.size);

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
	printk(KERN_INFO "%s: dma hw buffer:%016llx len:%Zu\n", cardName, (unsigned long long)sisdevice->dmablock.dma_addr, sisdevice->dmablock.size);
#else
	printk(KERN_INFO "%s: dma hw buffer:%08x len:%Zu\n", cardName, sisdevice->dmablock.dma_addr, sisdevice->dmablock.size);
#endif

	printk(KERN_INFO "%s: claiming irq line: %i\n", cardName, sisdevice->pdev->irq);
	res = request_irq(sisdevice->pdev->irq,
		(void *)&sis8300_isr,
		IRQF_SHARED,
		"sis8300",
		sisdevice);
	if(res != 0){
		printk(KERN_ALERT "%s: can't request isr handler\n", cardName);
		goto err_devcreate;
	}

	init_waitqueue_head(&sisdevice->intr_wait);
	sisdevice->intr_flag = 0;

	/* user irq setup */
	init_waitqueue_head(&sisdevice->usr_irq_wait);
	sisdevice->usr_irq_flag = 0;
	
	/* daq irq setup */
	init_waitqueue_head(&sisdevice->daq_done_irq_wait);
	sisdevice->daq_done_irq_flag = 0;

	printk(KERN_INFO "%s: Module Firmware/Revision:%08X\n", cardName, SIS8300REGREAD(sisdevice, SIS8XXX_IDENTIFIER_VERSION_REG));

	return 0;

err_devcreate:
	cdev_del(sisdevice->drvcdev);

err_cdevadd:
	unregister_chrdev_region(sisdevice->drvnum, 1);

err_chrdev:
	class_destroy(sisdevice->drvclass);

err_class:
	pci_iounmap(pdev, sisdevice->bar0);

err_iomap:
	pci_release_regions(pdev);

err_reqregions:
	pci_disable_device(pdev);

err_endevice:
	return -1;
}

/***************************************************************/
//static void __devexit exit_sis8300(struct pci_dev *pdev)
static void exit_sis8300(struct pci_dev *pdev)
{
	sis8300_dev *sisdevice;

	sisdevice = pci_get_drvdata(pdev);

	free_irq(sisdevice->pdev->irq, sisdevice);
	device_destroy(sisdevice->drvclass, sisdevice->drvnum);
	cdev_del(sisdevice->drvcdev);
	unregister_chrdev_region(sisdevice->drvnum, 1);
	class_destroy(sisdevice->drvclass);
	pci_free_consistent(pdev,
		      sisdevice->dmablock.size,
		      sisdevice->dmablock.data,
		      sisdevice->dmablock.dma_addr);
	pci_iounmap(pdev, sisdevice->bar0);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	kfree(sisdevice);
}

/***************************************************************/

/*
 * PCI Driver Struct
 */

static struct pci_driver sis8300_driver = {
	.name = "sis8300",
	.id_table = sis8300_id,
	.probe = init_sis8300,
	.remove = exit_sis8300,
};

/***************************************************************/

static int __init sis8300_module_init(void)
{
	int res;
	int i;

	printk(KERN_INFO "sis8300: loading driver ...\n");
	printk(KERN_INFO "sis8300: registering for following devices:\n");
	for(i = 0;i < (sizeof(sis8300_id) / sizeof(struct pci_device_id)) - 1;i++){
	  printk(KERN_INFO "sis8300: Vendor/Device: %04X/%04X\n", sis8300_id[i].vendor, sis8300_id[i].device);
	}	  
	printk(KERN_INFO "sis8300: Driver Version: v%d.%d (c)"DRIVER_VENDOR" "DRIVER_DATE"\n", DRIVER_MAJOR, DRIVER_MINOR);

	res = pci_register_driver(&sis8300_driver);
	if(res){
                printk(KERN_ERR "sis8300: can't register pci driver: %d\n", res);
	        return res;
        }

	return 0;
}

/***************************************************************/

static void __exit sis8300_module_exit(void)
{
	pci_unregister_driver(&sis8300_driver);

	printk(KERN_INFO "sis8300: driver unloaded\n");
}

/***************************************************************/

module_init(sis8300_module_init);
module_exit(sis8300_module_exit);

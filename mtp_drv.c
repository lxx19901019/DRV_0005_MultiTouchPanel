#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>

#define MTP_ADDR 0x70

#define  MTP_MAX_ID  1000 /*由硬件决定*/
#define  MTP_MAX_X   800
#define  MTP_MAX_Y   480

#define  MTP_IRQ  123

#define  MTP_NAME "XXXX"

struct input_dev *mtp_dev;
struct work_struct mtp_work;

irqreturn_t mtp_interrupt(int, void *)
{

	/* 本该获取触点数据，并上报
	 * 但是I2C是慢速设备，不该放在中断服务程序中操作
	 * 使用工作队列,让内核线程来操作
	 */

	/* 使用工作队列,让内核线程来操作 */

	schedule_work(&mtp_work);
	
	return IRQ_HANDLED;
}
static void mtp_work_func(struct work_struct*work)
{
	/*
	 * 读取I2C数据，获取触点数据并上报
	 *
	 *
	 */


	/* 上报 */
	/*完全松开时 input_mt_sync(mtp_dev)， input_sync(mtp_dev)*/
	if(/*没有触点*/) {
		input_mt_sync(mtp_dev);
		input_sync(mtp_dev);
		return;
	}
	for() {
		input_report_abs(mtp_dev, ABS_MT_POSITION_X, x);
		input_report_abs(mtp_dev, ABS_MT_POSITION_Y, y);
		input_report_abs(mtp_dev, ABS_MT_TRACKING_ID, id);
		input_mt_sync(mtp_dev);
	}
	input_sync(mtp_dev);

	

} 

static int __devinit mtp_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 输入子系统 */
	/* 分配input_dev */
	mtp_dev = input_allocate_device();

	/* 设置 */ 
	/* 2.1 能产生哪类事件 */
	set_bit(EV_ABS, mtp_dev->evbit);
	set_bit(EV_SYN, mtp_dev->evbit);
	/* 2.2 能产生这类事件中的哪些 */
	set_bit(ABS_MT_TRACKING_ID, mtp_dev->absbit);
	set_bit(ABS_MT_POSITION_X, mtp_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, mtp_dev->absbit);
	/* 2.3 这些事件的范围 */
	/*电容屏的最大ID值与"N点触摸"无关, 由触摸IC决定*/
	input_set_abs_params(mtp_dev, ABS_MT_TRACKING_ID, 0, MTP_MAX_ID,0 ,0);
	input_set_abs_params(mtp_dev, ABS_MT_POSITION_X, 0, MTP_MAX_X,0 ,0);
	input_set_abs_params(mtp_dev, ABS_MT_POSITION_Y, 0, MTP_MAX_Y,0 ,0);

	/* 要设置input_dev的name,android根据这个name找到配置文件 */
	mtp_dev->name = MTP_NAME; 
	
	/* 注册 */
	input_register_device(mtp_dev);
	/* 硬件相关 */
	INIT_WORK(&mtp_work,mtp_work_func);
	request_irq(MTP_IRQ, mtp_interrupt, IRQ_TYPE_EDGE_FALLING, "100ask_mtp", mtp_dev);				
	
	return 0;
}

static int __devexit mtp_remove(struct i2c_client *client)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	free_irq(MTP_IRQ,mtp_dev);
	cancel_work_sync(&mtp_work);
	input_unregister_device(mtp_dev);

	input_free_device(mtp_dev);
	return 0;
}

static const struct i2c_device_id mtp_id_table[] = {
	{ "100ask_mtp", 0 },
	{}
};

static int mtp_detect(struct i2c_client *client,
		       struct i2c_board_info *info)
{
	/* 能运行到这里, 表示该addr的设备是存在的
	 * 但是有些设备单凭地址无法分辨(A芯片的地址是0x50, B芯片的地址也是0x50)
	 * 还需要进一步读写I2C设备来分辨是哪款芯片
	 * detect就是用来进一步分辨这个芯片是哪一款，并且设置info->type
	 */
	
	printk("mtp_detect : addr = 0x%x\n", client->addr);

	/* 进一步判断是哪一款 */
	
	strlcpy(info->type, "100ask_mtp", I2C_NAME_SIZE);
	return 0;
	/**
	 *  返回0之后 会创建一个新设备
	 * i2c_new_device(adapter, &info) info->type = "100ask_mtp";
	 */
}

static const unsigned short addr_list[] = {MTP_ADDR, I2C_CLIENT_END };

/* 1. 分配/设置i2c_driver */
static struct i2c_driver mtp_driver = {
	.class  = I2C_CLASS_HWMON, /* 表示去哪些适配器上找设备 */
	.driver	= {
		.name	= "100ask",
		.owner	= THIS_MODULE,
	},
	.probe		= mtp_probe,
	.remove		= __devexit_p(mtp_remove),
	.id_table	= mtp_id_table,
	.detect     = mtp_detect,  /* 用这个函数来检测设备确实存在 */
	.address_list	= addr_list,   /* 这些设备的地址 */
};

static int mtp_drv_init(void)
{
	/* 2. 注册i2c_driver */
	i2c_add_driver(&mtp_driver);
	
	return 0;
}

static void mtp_drv_exit(void)
{
	i2c_del_driver(&mtp_driver);
}


module_init(mtp_drv_init);
module_exit(mtp_drv_exit);
MODULE_LICENSE("GPL");



/*
 * @Description: 
 * @Version: 2.0
 * @Autor: ZZT
 * @Date: 2022-01-28 19:36:18
 * @LastEditors: ZZT
 * @LastEditTime: 2022-01-29 11:13:27
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/* 使用了设备文件操作接口 */
#include <linux/fs.h>

/* 在此使用设备树模式的gpio子系统 */
#include <linux/of.h>
#include <linux/gpio/consumer.h>

/* 此驱动为虚拟平台总线驱动 */
#include <linux/platform_device.h>

/* 此驱动为misc类型驱动 */
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

/* 蜂鸣器设备结构体 */
struct my_beep_gpio_dev {
	/* misc驱动结构体 */
	struct miscdevice miscdev;
	/* gpio结构体 */
	struct gpio_desc *gpio;
};

/* 申明一个beep蜂鸣器驱动结构体 */
/* 也可以在初始化函数中申请 */
static struct my_beep_gpio_dev *my_beep_dev_data;

static ssize_t mybeep_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t mybeep_write(struct file *filp, const char __user *buf,
			    size_t cnt, loff_t *offt)
{
	int ret;
	char kern_buf[5];
	/* 得到应用层传递过来的数据 */
	ret = copy_from_user(kern_buf, buf, cnt);
	if (0 > ret) {
		printk(KERN_ERR
		       "mybeep: Failed to copy data from user buffer\r\n");
		return -EFAULT;
	}
	if ('0' == kern_buf[0]) {
		printk(KERN_INFO "beep get info 0\r\n");
		gpiod_set_value_cansleep(my_beep_dev_data->gpio, 0);
	} else {
		printk(KERN_INFO "beep get info not 0\r\n");
		gpiod_set_value_cansleep(my_beep_dev_data->gpio, 1);
	}
	return cnt;
}

static struct file_operations mybeep_fops = {
	.owner = THIS_MODULE,
	.open = mybeep_open,
	.write = mybeep_write,
};

static int my_beep_probe(struct platform_device *pdev)
{
	/* struct my_beep_gpio_dev *my_beep_dev_data; */
	dev_info(&pdev->dev, "BEEP driver has been probe!\n");
	my_beep_dev_data =
		devm_kzalloc(&pdev->dev, sizeof(*my_beep_dev_data), GFP_KERNEL);
	if (!my_beep_dev_data)
		return -ENOMEM;

	my_beep_dev_data->gpio =
		devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
	if (IS_ERR(my_beep_dev_data->gpio))
		return PTR_ERR(my_beep_dev_data->gpio);

	my_beep_dev_data->miscdev.name = "beep_by_zzt";
	my_beep_dev_data->miscdev.minor = MISC_DYNAMIC_MINOR;
	my_beep_dev_data->miscdev.fops = &mybeep_fops;

	platform_set_drvdata(pdev, my_beep_dev_data);

	return misc_register(&(my_beep_dev_data->miscdev));
}

static int my_beep_remove(struct platform_device *pdev)
{
	struct my_beep_gpio_dev *beep_data = pdev->dev.driver_data;
	dev_info(&pdev->dev, "BEEP driver has been removed!\n");
	misc_deregister(&(beep_data->miscdev));
	return 0;
}

/* 设备树如下 */
/*
beep {
	compatible = "gpio-beep-by-zzt";
	gpios = <&gpio0 60 GPIO_ACTIVE_HIGH>;
};
*/

/* 匹配列表 */
static const struct of_device_id beep_of_match[] = {
	{ .compatible = "gpio-beep-by-zzt" },
	{ /* Sentinel */ }
};

static struct platform_driver mybeep_driver = {
	.driver={
		.name = "gpio-beep-by-zzt",
		.of_match_table = beep_of_match,
	},
	.probe = my_beep_probe,
	.remove = my_beep_remove,
};

module_platform_driver(mybeep_driver);

MODULE_AUTHOR("zzttzz <2775859312@qq.com>");
MODULE_DESCRIPTION("Generic GPIO beeper driver");
MODULE_LICENSE("GPL");

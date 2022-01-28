/*
 * @Description: 
 * @Version: 2.0
 * @Autor: ZZT
 * @Date: 2022-01-28 19:36:18
 * @LastEditors: ZZT
 * @LastEditTime: 2022-01-28 20:09:10
 */
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>

/* 在此使用设备树模式的gpio子系统 */
#include <linux/of.h>
#include <linux/gpio/consumer.h>

/* 此驱动为虚拟平台总线驱动 */
#include <linux/platform_device.h>

/* 此驱动为misc类型驱动 */
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

/* 蜂鸣器设备结构体 */
struct my_beep_gpio_dev{
	/* misc驱动结构体 */
	struct miscdevice miscdev;
	/* gpio编号 */
	int gpio;
};

/* 申明一个beep蜂鸣器驱动结构体 */
/* 也可以在初始化函数中申请 */
/* struct my_beep_gpio_dev my_beep_dev_data; */

static int my_beep_probe(struct platform_device *pdev)
{
	struct my_beep_gpio_dev *beep_data;

	// beep_data = devm_kzalloc(&pdev->dev, sizeof(*beep_data), GFP_KERNEL);
	// if (!beep_data)
	// 	return -ENOMEM;

	// beep_data->gpio = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
	// if (IS_ERR(beep_data->gpio))
	// 	return PTR_ERR(beep_data->gpio);
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
	{ .compatible = "alientek,beeper" },
	{ /* Sentinel */ }
};

static struct platform_driver mybeep_driver = {
	.driver={
		.name = "gpio-beep-by-zzt",
		.of_match_table = beep_of_match,
	},
	.probe = my_beep_probe,
};

module_platform_driver(mybeep_driver);

MODULE_AUTHOR("zzttzz <2775859312@qq.com>");
MODULE_LICENSE("GPL");

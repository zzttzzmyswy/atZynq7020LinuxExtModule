/*
 * @Description: 
 * @Version: 2.0
 * @Autor: ZZT
 * @Date: 2022-01-29 11:49:47
 * @LastEditors: ZZT
 * @LastEditTime: 2022-01-30 09:02:52
 */
/*
 * @Description: 
 * @Version: 2.0
 * @Autor: ZZT
 * @Date: 2022-01-28 15:22:59
 * @LastEditors: ZZT
 * @LastEditTime: 2022-01-29 11:13:45
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init hello_init(void)
{
	int a = 100;
	float b = 120;
	printk("Hello word %f",b/a);
	return 0;
}
static void __exit hello_exit(void)
{
	printk("Goodbye world");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");

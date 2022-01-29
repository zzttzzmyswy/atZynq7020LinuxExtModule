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

static int hello_init(void)
{
	printk("Hello word");
	return 0;
}
static void hello_exit(void)
{
	printk("Goodbye world");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");

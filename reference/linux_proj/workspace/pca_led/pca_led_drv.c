/*
 * PCA9555 LED 驱动 — 仿 leds-gpio 的设备树模型
 *
 * 用法:
 *   echo 1 > /dev/pca_led   点亮 (PCA9555 P04 输出低)
 *   echo 0 > /dev/pca_led   熄灭 (PCA9555 P04 输出高，默认状态)
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

static struct gpio_desc *led_gpiod;
static dev_t             dev_num;
static struct class     *led_class;
static struct cdev       led_cdev;

/* ── write: 用户写 '1' / '0' ── */
static ssize_t pca_led_write(struct file *file, const char __user *buf,
                             size_t count, loff_t *ppos)
{
    char val;

    if (copy_from_user(&val, buf, 1))
        return -EFAULT;

    if (val == '1')
        gpiod_set_value(led_gpiod, 0);   /* active → 物理低电平 → LED 亮 */
    else if (val == '0')
        gpiod_set_value(led_gpiod, 1);   /* inactive → 物理高电平 → LED 灭 */
    else
        return -EINVAL;

    return count;
}

static const struct file_operations pca_led_fops = {
    .owner = THIS_MODULE,
    .write = pca_led_write,
};

/* ── probe: 从设备树取 GPIO，注册字符设备 ── */
static int pca_led_probe(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;

    /* 从设备树的 "led-gpios" 属性拿 GPIO，初始值 = 高电平 (灭) */
    led_gpiod = devm_gpiod_get(dev, "led", GPIOD_OUT_HIGH);
    if (IS_ERR(led_gpiod)) {
        dev_err(dev, "Failed to get led-gpios\n");
        return PTR_ERR(led_gpiod);
    }

    /* 分配主设备号 */
    ret = alloc_chrdev_region(&dev_num, 0, 1, "pca_led");
    if (ret) {
        dev_err(dev, "alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&led_cdev, &pca_led_fops);
    ret = cdev_add(&led_cdev, dev_num, 1);
    if (ret) {
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    /* 自动创建 /dev/pca_led */
    led_class = class_create(THIS_MODULE, "pca_led_class");
    if (IS_ERR(led_class)) {
        cdev_del(&led_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(led_class);
    }
    device_create(led_class, NULL, dev_num, NULL, "pca_led");

    dev_info(dev, "pca_led probed, major=%d, minor=%d\n",
             MAJOR(dev_num), MINOR(dev_num));
    dev_info(dev, "  usage: echo 1 > /dev/pca_led   (LED ON)\n");
    dev_info(dev, "         echo 0 > /dev/pca_led   (LED OFF)\n");

    return 0;
}

/* ── remove: 注销设备 ── */
static int pca_led_remove(struct platform_device *pdev)
{
    device_destroy(led_class, dev_num);
    class_destroy(led_class);
    cdev_del(&led_cdev);
    unregister_chrdev_region(dev_num, 1);
    dev_info(&pdev->dev, "pca_led removed\n");
    return 0;
}

/* ── 设备树匹配表 ── */
static const struct of_device_id pca_led_of_match[] = {
    { .compatible = "mosyu,pca-led" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, pca_led_of_match);

/* ── platform 驱动结构 ── */
static struct platform_driver pca_led_driver = {
    .probe  = pca_led_probe,
    .remove = pca_led_remove,
    .driver = {
        .name           = "pca_led",
        .of_match_table = pca_led_of_match,
    },
};

module_platform_driver(pca_led_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("mosyu");
MODULE_DESCRIPTION("PCA9555 LED character driver");

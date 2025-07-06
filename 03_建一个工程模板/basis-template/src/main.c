#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* 1000 msec = 1 sec */
#define  SLEEP_TIME_MS  1000
#define  LIGHT_TIME_MS  500

/*
 * 设备树 (DeviceTree) 宏定义
 * 使用 DT_ALIAS() 从设备树中获取节点的别名。
 * 我们需要4个LED和4个按钮。
 */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

#define SW0_NODE DT_ALIAS(sw0) // sw0 是 button0 的另一个常用别名
#define SW1_NODE DT_ALIAS(sw1)
#define SW2_NODE DT_ALIAS(sw2)
#define SW3_NODE DT_ALIAS(sw3)

// 使用 GPIO_DT_SPEC_GET 宏从设备树节点中提取 GPIO 的完整规格（控制器、引脚号、标志位）
// 这是一个更现代、更安全的做法。
// A build error on this line means your board is unsupported.
// See the sample documentation for information on how to fix it.
static const struct gpio_dt_spec leds[] = {
    GPIO_DT_SPEC_GET(LED0_NODE, gpios),
    GPIO_DT_SPEC_GET(LED1_NODE, gpios),
    GPIO_DT_SPEC_GET(LED2_NODE, gpios),
    GPIO_DT_SPEC_GET(LED3_NODE, gpios),
};

static const struct gpio_dt_spec buttons[] = {
    GPIO_DT_SPEC_GET(SW0_NODE, gpios),
    GPIO_DT_SPEC_GET(SW1_NODE, gpios),
    GPIO_DT_SPEC_GET(SW2_NODE, gpios),
    GPIO_DT_SPEC_GET(SW3_NODE, gpios),
};


// 定义一个静态的回调结构体，用于处理按钮中断
static struct gpio_callback button_cb_data;

// 定义按钮中断的回调函数
// 当任何一个配置了中断的按钮被按下时，这个函数就会被调用
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    // 遍历所有按钮，检查是哪一个触发了中断
    for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
        // (1 << buttons[i].pin) 创建一个掩码，检查触发中断的引脚是否是当前按钮的引脚
        if (pins & (1 << buttons[i].pin)) {
            printk("Button %d pressed!\n", i + 1);
            // 切换对应位置LED的状态
            gpio_pin_toggle_dt(&leds[i]);
        }
    }
}


// 大部分工作放在了主函数
int main(void) {
    int ret;
    printk("nRF5340 DK Button-LED demo started!\n");

    // 1. 配置所有LED引脚为输出模式
    for (int i = 0; i < ARRAY_SIZE(leds); i++) {
        if (!device_is_ready(leds[i].port)) {
            // 这个检查相当有官方样例中的 `if (!gpio_is_ready_dt(&led)) { ... }`
            printk("Error: LED device %s is not ready\n", leds[i].port->name);
            return 0;
        }
        //
        // 先把灯都关掉
        ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
        if (ret != 0) {
            printk("Error %d: failed to init (set to inactive) LED %d pin %d\n", ret, i, leds[i].pin);
            return 0;
        }
    }

    // 2. 配置所有按钮引脚为输入模式
    for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
        if (!device_is_ready(buttons[i].port)) {
            printk("Error: Button device %s is not ready\n", buttons[i].port->name);
            return 0;
        }
        //
        // 配置为输入模式
        ret = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
        if (ret != 0) {
            printk("Error %d: failed to setup (set to input mode) Button %d pin %d\n", ret, i, buttons[i].pin);
            return 0;
        }
        //
        // 配置中断，当引脚变为高电平（按钮按下）时触发
        ret = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_EDGE_TO_ACTIVE);
        if (ret != 0) {
            printk("Error %d: failed to setup (Button) interrupt on pin %d\n", ret, buttons[i].pin);
            return 0;
        }
    }

    // 3. 设置中断回调函数
    // 初始化回调结构体，指定回调函数和要监控的引脚掩码
    // 注意：这里只使用第一个按钮的控制器(port)来添加回调，因为nRF5340 DK上所有按钮都在同一个GPIO控制器上。
    // 如果按钮在不同的控制器上，需要为每个控制器单独添加回调。
    uint32_t pin_mask = 0;
    for(int i = 0; i < ARRAY_SIZE(buttons); i++) {
        // 把每一个按钮对应的引脚号，在 pin_mask 中对应的 bit 位置为 1
        pin_mask |= (1 << buttons[i].pin);
    }
    gpio_init_callback(&button_cb_data, button_pressed, pin_mask);

    // 将回调函数添加到GPIO驱动中
    gpio_add_callback(buttons[0].port, &button_cb_data);

    printk("Configuration complete. Press a button...\n");

    // 主循环可以为空，因为所有工作都由中断驱动
    return 0;
}

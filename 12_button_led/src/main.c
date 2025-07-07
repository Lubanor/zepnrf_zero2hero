#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* 1000 msec = 1 sec */
#define  SHORT_DELAY  100

// 使用 DT_ALIAS() 从设备树中获取节点的别名。
#define  LED0_NODE  DT_ALIAS(led0)
#define  LED1_NODE  DT_ALIAS(led1)
#define  LED2_NODE  DT_ALIAS(led2)
#define  LED3_NODE  DT_ALIAS(led3)

#define  SW0_NODE   DT_ALIAS(sw0) // sw0 是 button0 的别名
#define  SW1_NODE   DT_ALIAS(sw1)
#define  SW2_NODE   DT_ALIAS(sw2)
#define  SW3_NODE   DT_ALIAS(sw3)


// 使用 GPIO_DT_SPEC_GET 宏从设备树节点中提取 GPIO 的完整规格（控制器、引脚号、标志位）
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

///////////////////////////////////////////////////////////////////
// LED 相关的函数
int init_LED() {

	for (int i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!device_is_ready(leds[i].port)) {
			printk("Error: LED device %s is not ready!\n", leds[i].port->name);
			return -11;
		}

		// 先把灯都关掉
		int ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret != 0) {
			printk("Error %d: failed to init (set to inactive) LED %d pin %d\n", ret, i, leds[i].pin);
			return -12;
		}
	}

	return 0;
}

int turn_on_LED(int i) {
	if (i < 0) {return -13;}
	if (i >= ARRAY_SIZE(leds)) { return -13; }
	return gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
}

int turn_off_LED(int i) {
	if (i < 0) {return -13;}
	if (i >= ARRAY_SIZE(leds)) { return -13; }
	return gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
}

///////////////////////////////////////////////////////////////////
// 开关相关的函数
int init_button() {

	for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
		if (!device_is_ready(buttons[i].port)) {
			printk("Error: button device %s is not ready\n", buttons[i].port->name);
			return -21;
		}

		// 配置为输入模式，上拉电阻(根据硬件连接可能需要调整)
		int ret = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT | GPIO_PULL_UP);
		if (ret != 0) {
			printk("Error %d: failed to configure button %d pin %d\n", ret, i, buttons[i].pin);
			return -22;
		}
	}

	return 0;
}

int read_button_state(int i) {
	if (i < 0) { return -23; }
	if (i >= ARRAY_SIZE(buttons)) { return -23; }
	return gpio_pin_get_dt(&buttons[i]);
}

// =============================================================

int main() {
	int ret1, ret2;

	printk("nRF5340 DK button-LED starts!\n");
	ret1 = init_LED();
	ret2 = init_button();
	if (ret1 * ret2 == 0) {
		printk("Ready: all devices are ready to go!\n");
	} else {
		printk("Error: not all devices are good to go! Quitting...\n");
		return -1;
	}

	////////////////////////////////////////////////////
	while (1) {
		for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
			int val = read_button_state(i);

			if (val < 0) {
				printk("Error reading button %d\n", i);
				continue;
			}

			// 根据按键状态控制LED
			// 按下时灯亮; 会自动弹起, 弹起后灯亮
			if (val == 0) { // 弹起为0
				gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
			} else { // 按下为1
				gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
			}

			k_msleep(SHORT_DELAY);
		}
	}

	return 0;
}

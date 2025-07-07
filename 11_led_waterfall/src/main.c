#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* 1000 msec = 1 sec */
#define  SLEEP_TIME_MS  1000
#define  LIGHT_TIME_MS  200

/* The devicetree node identifier for the ledx alias */
#define  LED0_NODE  DT_ALIAS(led0)
#define  LED1_NODE  DT_ALIAS(led1)
#define  LED2_NODE  DT_ALIAS(led2)
#define  LED3_NODE  DT_ALIAS(led3)

/* A build error on this line means your board is unsupported */
static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(LED0_NODE, gpios),
	GPIO_DT_SPEC_GET(LED1_NODE, gpios),
	GPIO_DT_SPEC_GET(LED2_NODE, gpios),
	GPIO_DT_SPEC_GET(LED3_NODE, gpios),
};

int turn_on_LED(int i) {
	if (i < 0) {return -1;}
	if (i >= ARRAY_SIZE(leds)) {return -1;}
	int ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
	return ret;
}

int turn_off_LED(int i) {
	if (i < 0) {return -1;}
	if (i >= ARRAY_SIZE(leds)) {return -1;}
	int ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
	return ret;
}

// =============================================================

int main(void) {
	int ret = 0;

	printk("nRF5340 DK LED-waterfall starts!\n");
	for (int i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!device_is_ready(leds[i].port)) {
			printk("Error: LED device %s is not ready!\n", leds[i].port->name);
			return -2;
		}

		// 先把灯都关掉
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret != 0) {
			printk("Error %d: failed to init (set to inactive) LED %d pin %d\n", ret, i, leds[i].pin);
			return -3;
		}
	}

	printk("Ready: all LED devices are ready to go!\n");
	////////////////////////////////////////////////////

	while (1) {
		for (int i = 0; i < ARRAY_SIZE(leds); i++) {
			if (turn_on_LED(i) < 0) {
				printk("Error turning on LED %d\n", i);
				continue;
			}

			k_msleep(LIGHT_TIME_MS);

			if (turn_off_LED(i) < 0) {
				printk("Error turning off LED %d\n", i);
				continue;
			}

			k_msleep(SLEEP_TIME_MS);
		}
	}

	return ret;
}

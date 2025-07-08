#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <nrfx_timer.h>

/* LED定义 */
// 使用 GPIO_DT_SPEC_GET 宏从设备树节点中提取 GPIO 的完整规格（控制器、引脚号、标志位）
// 使用 DT_ALIAS() 从设备树中获取节点的别名
#define LED_NUM 4
static const struct gpio_dt_spec leds[LED_NUM] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios),
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
	if (i < 0) { return -13; }
	if (i >= LED_NUM) { return -13; }
	return gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
}

int turn_off_LED(int i) {
	if (i < 0) { return -13; }
	if (i >= LED_NUM) { return -13; }
	return gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
}

int toggle_LED(int i) {
	if (i < 0) { return -13; }
	if (i >= LED_NUM) { return -13; }
	int val = gpio_pin_get_dt(&leds[i]);
	return gpio_pin_configure_dt(&leds[i], 1-val);
}

///////////////////////////////////////////////////////////////////
// This section is stolen from https://github.com/too1/ncs-nrfx-timer-example
//
// Get a reference to the TIMER1 instance
#define TIMER_ID 1
static const nrfx_timer_t my_timer = NRFX_TIMER_INSTANCE(TIMER_ID);

// 中断callback!
void timer_event_handler(nrf_timer_event_t event_type, void * p_context) {
	static int current_led = 0;  // 当前点亮的LED索引

	switch(event_type) {
		case NRF_TIMER_EVENT_COMPARE0:
			for (int i = 0; i < LED_NUM; i++) { turn_off_LED(i); } // 先关闭所有LED
			turn_on_LED(current_led); // 点亮当前LED
			current_led = (current_led + 1) % LED_NUM; // 更新到下一个LED（循环）
			printk("Current LED: %d\n", current_led);
			break;
        default: break;
	}
}

// Function for initializing the TIMER1 peripheral using the nrfx driver
static void timer_init() {
	nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG(1000000); // 定时器的基础频率
	timer_config.bit_width = NRF_TIMER_BIT_WIDTH_32;

	int err = nrfx_timer_init(&my_timer, &timer_config, timer_event_handler);
	if (err != NRFX_SUCCESS) {printk("Error initializing timer: %x\n", err);}

	// 绕过Zephyr的中断封装层, 直接将中断服务例程（ISR）绑定到指定的硬件中断线
	IRQ_DIRECT_CONNECT(TIMER1_IRQn, 0, nrfx_timer_1_irq_handler, 0);
	irq_enable(TIMER1_IRQn);
}

// Function for scheduling repeated callbacks from TIMER1
// 用来控制定时器“每隔多久触发一次中断”, 修改其参数即可实时调整LED闪烁速度
// 便于动态调整!
static void timer_repeated_timer_start(uint32_t timeout_us) {
	nrfx_timer_enable(&my_timer); // 启动定时器硬件, 开始计数
	nrfx_timer_extended_compare(
		&my_timer, NRF_TIMER_CC_CHANNEL0, timeout_us, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true
	);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
	printk("nRF5340 DK button-LED starts!\n");

	// 初始化LED
	if (init_LED() != 0) {
		printk("Error: not all LEDs are good to go! Quitting...\n");
		return -1;
	}

	// Initialize TIMER1
	timer_init();
	// Setup TIMER1 to generate callbacks every 0.75 seconds
	timer_repeated_timer_start(750000);

	// 主线程进入休眠，由中断处理所有工作
	while (1) { k_sleep(K_FOREVER); } // 不加这句也行, 退出main()后, 这里设置的中断响应仍然可以正常运行

	return 0;
}

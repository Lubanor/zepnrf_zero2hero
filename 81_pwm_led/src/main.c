#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <math.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* 使用DT_NODELABEL替代DT_ALIAS */
#define PWM_LED0_NODE DT_NODELABEL(pwm_led0)
#define PWM_LED1_NODE DT_NODELABEL(pwm_led1)
#define PWM_LED2_NODE DT_NODELABEL(pwm_led2)
#define PWM_LED3_NODE DT_NODELABEL(pwm_led3)

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(PWM_LED0_NODE);
static const struct pwm_dt_spec pwm_led1 = PWM_DT_SPEC_GET(PWM_LED1_NODE);
static const struct pwm_dt_spec pwm_led2 = PWM_DT_SPEC_GET(PWM_LED2_NODE);
static const struct pwm_dt_spec pwm_led3 = PWM_DT_SPEC_GET(PWM_LED3_NODE);

#define PWM_PERIOD_NS 20000000  // 20ms period (50Hz)

/* 提前声明所有效果函数 */
void update_led_duty_cycle(const struct pwm_dt_spec *led, uint32_t duty_cycle_ns);
void breathing_effect(const struct pwm_dt_spec *led, uint32_t period_ms);
void blinking_effect(const struct pwm_dt_spec *led, uint32_t period_ms);
void variable_blink_effect(const struct pwm_dt_spec *led, uint32_t base_period_ms);
void smooth_transition_effect(const struct pwm_dt_spec *led, uint32_t period_ms);

void update_led_duty_cycle(const struct pwm_dt_spec *led, uint32_t duty_cycle_ns) {
	if (!pwm_is_ready_dt(led)) {
		LOG_ERR("PWM device %s is not ready", led->dev->name);
		return;
	}

	int ret = pwm_set_pulse_dt(led, duty_cycle_ns);
	if (ret < 0) {
		LOG_ERR("Failed to set PWM pulse width for %s: %d", led->dev->name, ret);
	}
}

void breathing_effect(const struct pwm_dt_spec *led, uint32_t period_ms) {
	static uint32_t count = 0;
	static bool increasing = true;
	static uint32_t last_update = 0;
	uint32_t now = k_uptime_get_32();
	uint32_t delta = 20000;

	if (now - last_update < period_ms / 100) {
		return;
	}
	last_update = now;

	if (increasing) {
		count += delta;
		if (count >= PWM_PERIOD_NS) {
			count = PWM_PERIOD_NS;
			increasing = false;
		}
	} else {
		count -= delta;
		if (count <= 0) {
			count = 0;
			increasing = true;
		}
	}

	update_led_duty_cycle(led, count);
}

void blinking_effect(const struct pwm_dt_spec *led, uint32_t period_ms) {
	static uint32_t last_toggle = 0;
	static bool is_on = false;
	uint32_t now = k_uptime_get_32();

	if (now - last_toggle < period_ms) {
		return;
	}
	last_toggle = now;

	is_on = !is_on;
	update_led_duty_cycle(led, is_on ? PWM_PERIOD_NS : 0);
}

void variable_blink_effect(const struct pwm_dt_spec *led, uint32_t base_period_ms) {
	static uint32_t phase = 0;
	static uint32_t last_update = 0;
	uint32_t now = k_uptime_get_32();

	if (now - last_update < base_period_ms / 10) {
		return;
	}
	last_update = now;

	phase = (phase + 1) % 100;
	uint32_t duty_cycle = PWM_PERIOD_NS * (phase < 50 ? phase : 100 - phase) / 50;
	update_led_duty_cycle(led, duty_cycle);
}

void smooth_transition_effect(const struct pwm_dt_spec *led, uint32_t period_ms) {
	static uint32_t position = 0;
	static uint32_t last_update = 0;
	uint32_t now = k_uptime_get_32();

	if (now - last_update < period_ms / 360) {
		return;
	}
	last_update = now;

	position = (position + 1) % 360;
	float radians = position * 3.1415926f / 180.0f;
	uint32_t duty_cycle = (uint32_t)((sinf(radians) + 1.0f) * PWM_PERIOD_NS / 2.0f);
	update_led_duty_cycle(led, duty_cycle);
}

int main(void) {
	LOG_INF("PWM LED Control Started");

	if (!device_is_ready(pwm_led0.dev)) {
		LOG_ERR("PWM0 device not ready");
		return -1;
	} else if (!device_is_ready(pwm_led1.dev)) {
		LOG_ERR("PWM1 device not ready");
		return -1;
	} else if (!device_is_ready(pwm_led2.dev)) {
		LOG_ERR("PWM2 device not ready");
		return -1;
	} else if (!device_is_ready(pwm_led3.dev)) {
		LOG_ERR("PWM3 device not ready");
		return -1;
	} else {
		LOG_INF("all PWM (#0-3) devices are ready");
	}

	// 初始设置所有LED为关闭状态
	update_led_duty_cycle(&pwm_led0, 0);
	update_led_duty_cycle(&pwm_led1, 0);
	update_led_duty_cycle(&pwm_led2, 0);
	update_led_duty_cycle(&pwm_led3, 0);

	while (1) {
		// LED0: Breathing effect (slow)
		breathing_effect(&pwm_led0, 200);

		// LED1: Blinking effect (medium speed)
		blinking_effect(&pwm_led1, 1000);

		// LED2: Variable blink effect (fast changing)
		variable_blink_effect(&pwm_led2, 2000);

		// LED3: Smooth sine wave transition
		smooth_transition_effect(&pwm_led3, 4000);

		k_sleep(K_MSEC(10));
	}

	return 0;
}

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/display/cfb.h>
#include <string.h>
#include <math.h>
// #include <zephyr/drivers/i2c.h>
// #include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display_test, LOG_LEVEL_INF);

static const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
// const struct device *display = DEVICE_DT_GET(DT_NODELABEL(ssd1306));

void my_draw_point(const struct device *dev, uint16_t xx, uint16_t yy) {
	cfb_draw_point(dev, &(struct cfb_position){.x=xx, .y=yy});
}

// 滚动文本（修正版）
void cfb_scroll_text(const struct device *dev, const char *text, uint16_t speed_ms) {
	uint16_t width = cfb_get_display_parameter(dev, CFB_DISPLAY_WIDTH);
	uint16_t text_width = strlen(text) * 8; // 假设每个字符宽 8 像素
	int16_t x = width; // 从屏幕右侧开始

	for (int i=0; i<2*text_width; i++) {
		cfb_framebuffer_clear(dev, false);
		cfb_print(dev, text, x, 0); // 在 (x, 0) 打印文本
		cfb_framebuffer_finalize(dev);

		x--;
		if (x < -text_width) {
			x = width; // 循环滚动
		}
		k_msleep(speed_ms);
	}
}

// 动态波形（修正版）
void cfb_draw_waveform(const struct device *dev, int16_t *data, uint16_t len, uint16_t speed_ms) {
	while (1) {
		// 1. 清屏
		cfb_framebuffer_clear(dev, false);

		// 2. 绘制波形
		for (uint16_t i = 0; i < len - 1; i++) {
			struct cfb_position start = {.x = i, .y = 32 - data[i]};
			struct cfb_position end = {.x = i + 1, .y = 32 - data[i + 1]};
			cfb_draw_line(dev, &start, &end);
		}

		// 3. 刷新屏幕
		cfb_framebuffer_finalize(dev);
		// 到上面, 是画了一帧

		// 4. 动态更新数据（向左平移波形）
		int16_t first_value = data[0];
		for (uint16_t i = 0; i < len - 1; i++) {
			data[i] = data[i + 1]; // 左移数据
		}
		data[len - 1] = first_value; // 循环填充

		// 5. 控制刷新速度
		k_msleep(speed_ms);
	}
}

int main() {
	LOG_INF("Starting OLED Demo...");

	if (!device_is_ready(display)) {
		LOG_ERR("Error: Display device is not ready!");
		return -1;
	} else {
		LOG_INF("Display device is ready!");
	}

	int err = cfb_framebuffer_init(display);
	if (err != 0) {
		LOG_ERR("Error %d: Character Framebuffer (CFB) initialization failed!\n", err);
		return err;
	} else {
		LOG_INF("Character Framebuffer (CFB) initialized!");
	}

	LOG_INF("Turning on screen, and cleared the framebuffer...");
	display_blanking_off(display);  // 开启显示（可选）
	cfb_framebuffer_clear(display, true); // 清空屏幕 use false?
	LOG_INF("Waiting...");
	k_msleep(1000);

	// 示例：滚动文本
	cfb_scroll_text(display, "Hello, World~~~", 100);
	k_msleep(1000);

	// 示例：动态波形
	int16_t sine_wave[128];
	for (int i = 0; i < 128; i++) { sine_wave[i] = 20 * sin(i * 0.1); }
	// 启动动态波形（参数：设备指针、数据数组、数据长度、刷新速度ms）
	cfb_draw_waveform(display, sine_wave, 128, 50); // 50ms刷新一次

	return 0;
}

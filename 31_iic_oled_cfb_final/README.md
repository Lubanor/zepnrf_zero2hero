# OLED小屏 (iic方式)

### 功能描述
使用OLED小屏 -- ver1

### 硬件要求
- 开发板：nRF5340 DK（或其他兼容板）

- OLED小屏幕:
  + 尺寸: 0.96寸
  + 电压DC: 3.3V
  + 分辨率: 128×64
  + 显存GDDRAM: 128×8 Bytes SRAM
  + 驱动: SSD1306
  + 管脚数: 4针
  + 接口: IIC
  + 工作温度: -40℃ ~ +85℃

- 连接方式:
  + OLED的四个引脚依次是: VCC, GND, SCL, SDA
  + 依次将它们连接DK上的: VDD, GND, P1.03, P1.02
  + 不需要外接上拉电阻

### Build & Flash
```bash
cd ~/ncs
rm -rf build
bash

source zephyr/zephyr-env.sh
export ZEPHYR_SDK_INSTALL_DIR=$HOME/zephyr-tools/zephyr-sdk-0.16.1
west build -b nrf5340dk/nrf5340/cpuapp --shield ssd1306_128x64 myprojects/31_iic_oled
west flash
```

### 辅助调试
这个小东西, 调试起来可真不容易, 必须查看DK板子通过UART串口发回的信息.
以Linux为例:
```bash
ls /dev # 找到类似`ttyACMx`的设备
screen /dev/ttyACM1 115200 # 连接并查看回馈信息
# 退出screen: Ctrl-A k
```

之前由于缓冲区太小, 怎么尝试都不成功. 直到通过串口看到了相关提示, 才有的放矢地修正错误.

### Reference:
- https://github.com/zephyrproject-rtos/zephyr/tree/main/boards/shields/ssd1306
- https://github.com/zephyrproject-rtos/zephyr/tree/main/samples/subsys/display/lvgl
  + https://zephyr-docs.listenai.com/samples/subsys/display/lvgl/README.html
- https://github.com/ShaunLinTW/nRF5340-SSD1306-I2C
- https://github.com/hx91/oled-ssd1306-lvgl

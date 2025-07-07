# 流水灯

`11_led_waterfall`

## 功能描述
- 4 个 LED (间隔固定的延迟) 依次点亮

## 硬件要求
- 开发板：nRF5340 DK（或其他兼容板）

## build & flash
```bash
cd ~/ncs
rm -rf build

bash

source zephyr/zephyr-env.sh
export ZEPHYR_SDK_INSTALL_DIR=$HOME/zephyr-tools/zephyr-sdk-0.16.1
west build -b nrf5340dk/nrf5340/cpuapp myprojects/11_led_waterfall
```

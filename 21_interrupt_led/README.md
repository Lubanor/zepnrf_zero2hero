# 流水灯 (时钟中断方式)

## 功能描述
- 使用时钟中断, 点亮 4 个LED

## 硬件要求
- 开发板：nRF5340 DK（或其他兼容板）

## build & flash
```bash
cd ~/ncs
rm -rf build

bash

source zephyr/zephyr-env.sh
export ZEPHYR_SDK_INSTALL_DIR=$HOME/zephyr-tools/zephyr-sdk-0.16.1
west build -b nrf5340dk/nrf5340/cpuapp myprojects/21_interrupt_led
```

## Reference
- https://github.com/too1/ncs-nrfx-timer-example

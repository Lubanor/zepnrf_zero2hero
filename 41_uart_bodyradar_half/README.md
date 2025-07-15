# 人体检测雷达

### 功能描述
使用人体检测雷达模块 Rd03

### 硬件要求
- 开发板：nRF5340 DK (或其他兼容板)
- Rd03人体雷达
  + 引脚依次是: 3V3, GND, OT1(Tx), Rx, OT2(output)
    * 连接DK: VDD, GND, P1.14, P1.13, P1.15
    * 避免使用 UART0, 因为它默认用于调试日志输出, 连接 /dev/ttyACMx
  + 参考规格书

### Build & Flash
```bash
cd ~/ncs
rm -rf build
bash
source zephyr/zephyr-env.sh
export ZEPHYR_SDK_INSTALL_DIR=$HOME/zephyr-tools/zephyr-sdk-0.16.1
west build -b nrf5340dk/nrf5340/cpuapp myprojects/41_uart_bodyradar
west flash
```

### 辅助调试
DK板子通过UART串口发回的信息:
```bash
ls /dev # 找到类似`ttyACMx`的设备
screen /dev/ttyACM1 115200 # 连接并查看回馈信息
# 退出screen: Ctrl-A k
```

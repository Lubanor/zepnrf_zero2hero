### Programing `Nordic nRF5340` on `Zephyr RTOS` 学习笔记

- nRF5340 DK
- Manjaro Linux
- bash shell

---

```bash
# env
cd ~/ncs
rm -rf build
bash
#
source zephyr/zephyr-env.sh
export ZEPHYR_SDK_INSTALL_DIR=$HOME/zephyr-tools/zephyr-sdk-0.16.1

# # build & flash
# west build -b nrf5340dk/nrf5340/cpuapp myprojects/prj_name
# west flash

# debug
screen /dev/ttyACM1 115200
```

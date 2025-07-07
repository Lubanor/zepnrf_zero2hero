**CMakeLists.txt ** 是 Zephyr 项目的构建配置文件，定义如何编译代码、链接库、包含头文件等。
Zephyr 使用 **CMake** 作为构建系统，因此需要正确配置 `CMakeLists.txt`。

---

## **最简单的 CMakeLists.txt 示例**
```cmake
# 1. 声明 CMake 的最低版本要求（Zephyr 推荐 ≥3.20）
cmake_minimum_required(VERSION 3.20.0)

# 2. 定义项目名称（必须与目录名一致）
project(my_zephyr_app)

# 3. 加载 Zephyr 的构建系统，`$ENV{ZEPHYR_BASE}` 是 Zephyr 环境的路径（需提前设置）
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# 4. 告诉 Zephyr 把 `src/main.c` 编译进可执行文件 `app`（`PRIVATE` 表示仅当前目标使用）
target_sources(app PRIVATE src/main.c)
```

---

## **进阶配置**
### **(1) 添加多个源文件**
```cmake
target_sources(app PRIVATE
    src/main.c
    src/sensor.c
    src/ble_handler.c
)
```

### **(2) 包含自定义头文件目录**
```cmake
# 添加头文件搜索路径
target_include_directories(app PRIVATE
    include/    # 自定义头文件目录
)
```

### **(3) 链接外部库（如自定义静态库）**
```cmake
# 假设有一个 libcustom.a 需要链接
target_link_libraries(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libcustom.a
)
```

### **(4) 条件编译（根据板子选择代码）**
```cmake
if (CONFIG_BOARD_NRF5340DK_NRF5340_CPUAPP)
    target_sources(app PRIVATE src/board_specific_nrf53.c)
elseif (CONFIG_BOARD_STM32F4_DISCO)
    target_sources(app PRIVATE src/board_specific_stm32.c)
endif()
```

---

## **实战：nRF5340 的完整 CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.20.0)

project(my_nrf5340_app)

# 引入 Zephyr
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# 添加主代码
target_sources(app PRIVATE
    src/main.c
    src/ble.c
    src/sensors.c
)

# 包含自定义头文件
target_include_directories(app PRIVATE
    include/
)

# 启用 Zephyr 的日志系统
target_link_libraries(app PRIVATE
    zephyr_interface
)

# 条件编译：仅在启用蓝牙时编译 BLE 相关代码
if (CONFIG_BT)
    target_sources(app PRIVATE src/ble_handler.c)
    target_compile_definitions(app PRIVATE USE_BLE=1)
endif()
```

---

## **小结**
| 功能                | CMake 写法                          |
|---------------------|-------------------------------------|
| **定义项目**        | `project(my_app)`                   |
| **添加源文件**      | `target_sources(app PRIVATE src/a.c)` |
| **包含头文件**      | `target_include_directories(...)`   |
| **链接库**          | `target_link_libraries(...)`        |
| **条件编译**        | `if(CONFIG_FEATURE)`                |


---

此外,

### **如何查看 Zephyr 支持的变量？**
- 运行 `west build -b nrf5340dk/nrf5340/cpuapp -t menuconfig`，搜索相关选项。

### **如何添加自定义 CMake 模块？**
- 在项目里创建 `cmake/` 目录，存放 `.cmake` 文件，然后在 `CMakeLists.txt` 添加：
  ```cmake
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
  include(your_module.cmake)
  ```

### **如何调试 CMake 变量？**
- 在 `CMakeLists.txt` 里插入：
  ```cmake
  message(STATUS "MY_VAR = ${MY_VAR}")
  ```
  编译时会在终端打印该变量值。


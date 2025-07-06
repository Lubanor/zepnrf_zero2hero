.. zephyr:code-sample:: basis-template
   :name: basis-template
   :relevant-api: gpio_interface

   Light LED when corresponding button is pressed, using the GPIO API.


Overview
********

A template for button-controlled LEDs using the :ref:`GPIO API <gpio_api>`.

The source code shows how to:

1. Get button and LED pin specifications from the :ref:`devicetree <dt-guide>` as :c:struct:`gpio_dt_spec` structures
2. Configure GPIO pins as inputs (buttons) and outputs (LEDs)
3. Read button states and control corresponding LEDs
4. Handle button debouncing in software


- cd ~/ncs
- west build -b nrf5340dk/nrf5340/cpuapp myprojects/nrf5340-button-led
- west flash


Building and Running
********************

Build and flash as follows, changing ``reel_board`` for your board:

.. zephyr-app-commands::
   :zephyr-app: myprojects/basis-template
   :board: reel_board
   :goals: build flash
   :compact:


Adding board support
********************

To add support for your board, add something like this to your devicetree:

.. code-block:: DTS

   / {
       aliases {
           led0 = &led0;
           button0 = &button0;
       };

       leds {
           compatible = "gpio-leds";
           led0: led_0 {
               gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;
           };
       };

       buttons {
           compatible = "gpio-keys";
           button0: button_0 {
               gpios = <&gpio0 11 GPIO_ACTIVE_LOW>;
               label = "User button";
           };
       };
   };


Configuration notes:

- The above defines ``led0`` on pin 13 and ``button0`` on pin 11 of GPIO controller ``gpio0``
- :c:macro:`GPIO_ACTIVE_LOW` means:
  - For LEDs: The LED turns on when the pin is driven low
  - For buttons: The pin reads low when the button is pressed
- Multiple buttons/LEDs can be defined by adding more entries


How it runs
***********

- 启动与内核初始化: 当 nRF5340 上电后，它会先执行一些非常底层的启动代码。这些代码会初始化 CPU、时钟等基本硬件。

- 然后，Zephyr 内核开始接管。内核会初始化它自己需要的所有东西：调度器、内存管理、中断控制器等。

- 驱动初始化: 接着，内核会根据你的配置 (prj.conf) 和设备树 (.dts)，去初始化所有被启用的设备驱动程序。
- 在我们的例子中，GPIO 驱动程序就在这个阶段被初始化并准备就绪。

- main 函数作为第一个“线程”启动: 内核完成所有准备工作后，它会把你的 main 函数作为一个线程 (Thread) 来启动。
- 在 RTOS 的世界里，main 不再是整个程序的生命线，它仅仅是应用程序的入口线程或主线程 (Main Thread)。

- main 函数的使命: 你的 main 函数在这个阶段执行。它的核心使命通常是进行一次性的应用层配置。在我们的代码里，它：
    - 配置 LED 引脚为输出。
    - 配置 Button 引脚为输入和中断。
    - 调用 gpio_add_callback()，将 button_pressed 函数的地址和引脚掩码，注册到了 GPIO 驱动程序中。
    - 这一步至关重要！驱动程序现在“记住”了这个指令。

- main 函数的终结: 当 main 函数执行到 return 0; 时，主线程的任务完成了。
- 内核会回收这个线程所占用的资源（比如它的栈空间）。

- 内核永不眠 (The Kernel Never Sleeps): 这才是关键！
- 即使 main 线程已经结束，Zephyr 内核本身仍然在后台运行。
- 内核现在进入了一个无限循环，等待着外部事件的发生。这些事件就是中断 (Interrupts)。

- 中断发生: 当你按下按钮时：
    - GPIO 硬件检测到电平变化，向 CPU 发送一个中断信号。
    - CPU 立刻暂停当前所有工作，跳转到预先设置好的中断服务程序 (ISR)。这个 ISR 是 GPIO 驱动程序的一部分。
    - 驱动程序的 ISR 会快速处理硬件（如清除中断标志），然后检查是哪个引脚触发了中断，并找到需回调的函数。
    - 驱动程序会通知内核调度器：“嘿，我这里有一个回调函数 (button_pressed) 需要执行！”
    - 内核调度器会在合适的时机，调用我们之前注册的 button_pressed 函数。

# UART

- 与IIC类似, UART（Universal Asynchronous Receiver/Transmitter, 通用异步收发传输器）也是最基础串行通信协议之一.
- 而且它的通信是 **点对点连接** 的, 也即通信的双方只需简单的用两根线（TX发送和RX接收）互相对接即可 (VCC, GND另连).
- 而不像IIC那样, 需要一个专门的时钟信号线: 在UART中, 通信的双方只能依赖预先约定的波特率 (Baud Rate) 进行同步, 这也就是它被称为 **异步通信** 的原因.
- 也因此,异步通信对传输数据的格式有严格的要求:
   - 起始位（逻辑低电平）
   - 5-9个数据位
   - 可选的奇偶校验位
   - 1-2个停止位（逻辑高电平）

- UART 连线量少, 连接简单(不需考虑上拉电阻等), 支持广泛, 使用便捷, 调试方便, 因而容易被我们初学者所接受.

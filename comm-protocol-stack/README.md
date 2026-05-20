# 纯C通信协议栈 - Comm Protocol Stack

🕳️ 通用私有协议栈完整实现，从定协议格式 → 分层架构 → 编解码 → 收发引擎 → 状态机 → 异常处理 → 适配TCP/UART/UDP，一套能跑通工业通信的协议栈。

---

## 📐 协议帧格式

```
+--------+--------+--------+--------+--------+--------+--------+--------+
|  帧头  |  长度  | 地址   | 命令码 | 数据域(N字节)      | 校验   |  帧尾  |
| 2字节  | 2字节  | 1字节  | 1字节  | 0~65535字节        | 2字节  | 2字节  |
| AA BB  | LL HH  | 01-FF  | 01-FF  |                    | CRC16  | CC DD  |
+--------+--------+--------+--------+--------+--------+--------+--------+
```

| 字段   | 长度 | 说明                           |
|--------|------|--------------------------------|
| 帧头   | 2B   | 0xAA 0xBB，帧同步标识          |
| 长度   | 2B   | 整个帧不含头尾的长度（Big-End） |
| 地址   | 1B   | 设备从机地址 0x01-0xFF         |
| 命令码 | 1B   | 读/写/上报/心跳/ACK等          |
| 数据域 | N    | 实际业务数据，变长             |
| 校验   | 2B   | CRC16-Modbus                   |
| 帧尾   | 2B   | 0xCC 0xDD                      |

---

## 🏗️ 四层架构

```
┌─────────────────────────────────────────────────┐
│                 应用层 (App Layer)               │
│   业务指令：读寄存器、写参数、心跳上报、设备控制    │
└─────────────────────┬───────────────────────────┘
                      │ proto_stack_send()
┌─────────────────────▼───────────────────────────┐
│              协议封装层 (Protocol Layer)           │
│   组帧、解帧、CRC校验、大小端转换、帧合法性校验     │
└─────────────────────┬───────────────────────────┘
                      │ send_frame()
┌─────────────────────▼───────────────────────────┐
│           传输适配层 (Transport Adapter)           │
│   统一接口适配：TCP / UART / CAN / UDP           │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              底层驱动层 (Driver Layer)             │
│   硬件收发：串口、Socket、CAN驱动、中断接收       │
└─────────────────────────────────────────────────┘
```

---

## 📂 目录结构

```
comm-protocol-stack/
├── src/
│   ├── core/              # 核心组件
│   │   ├── ringbuffer.h/c # 环形缓冲区
│   │   └── crc.h/c       # CRC校验算法
│   ├── proto/             # 协议层
│   │   ├── proto_frame.h # 协议帧定义
│   │   ├── proto_parser.h/c  # 解包状态机
│   │   ├── proto_packer.h/c  # 封包组帧
│   │   ├── session.h/c    # 会话管理
│   │   └── proto_stack.h/c   # 协议栈核心
│   ├── transport/         # 传输适配层
│   │   ├── transport_uart.h/c   # UART适配
│   │   └── transport_socket.h/c # TCP/UDP适配
│   └── app/              # 应用层
├── example/
│   ├── example_uart.c    # UART示例
│   └── example_tcp.c     # TCP示例
└── docs/
    └── protocol_spec.md  # 协议详细规格书
```

---

## 🔧 核心组件

### 1. 环形缓冲区 (`ringbuffer.h`)

解决流式字节粘包问题，协议栈的灵魂组件：

```c
ringbuffer_t rb;
uint8_t buf[1024];
ringbuffer_init(&rb, buf, 1024);

// 写入数据
ringbuffer_write(&rb, data, len);

// 读取数据（不删除）
ringbuffer_read(&rb, out, len);

// 读取数据（删除已读）
ringbuffer_read_pop(&rb, out, len);

// 获取可用数据长度
uint16_t avail = ringbuffer_available(&rb);
```

### 2. CRC校验 (`crc.h`)

支持多种CRC算法：

```c
// CRC16-Modbus（默认协议校验）
uint16_t crc = crc16_modbus(data, len);

// CRC16-CCITT (XModem)
uint16_t crc = crc16_xmodem(data, len);

// CRC32
uint32_t crc = crc32_ethernet(data, len);

// 简单累加校验
uint8_t sum = checksum_sum8(data, len);
```

### 3. 解包状态机 (`proto_parser.h`)

字节流逐字节解析，自动解决粘包、半包：

```c
proto_parser_t parser;
proto_parser_init(&parser);

// 方式1：逐字节喂入
int ret = proto_parser_feed(&parser, byte);
if (ret == 0) {
    // 收到完整帧
    proto_frame_t *frame = &parser.frame;
}

// 方式2：从环形缓冲区解析
proto_frame_t frame;
ret = proto_parser_parse_rb(&parser, &rb, &frame);
if (ret == 0) {
    // 处理完整帧
}
```

**状态流转：**
```
WAIT_HEAD1 → WAIT_HEAD2 → WAIT_LEN1 → WAIT_LEN2 → 
WAIT_ADDR → WAIT_CMD → WAIT_DATA → WAIT_CRC1 → 
WAIT_CRC2 → WAIT_TAIL1 → WAIT_TAIL2 → COMPLETE
```

### 4. 封包组帧 (`proto_packer.h`)

发送端组装标准协议帧：

```c
uint8_t frame_buf[256];
uint16_t len = proto_pack(
    0x02,              // 目标地址
    PROTO_CMD_READ,    // 命令码
    data,              // 数据
    data_len,           // 数据长度
    frame_buf,         // 输出缓冲区
    sizeof(frame_buf)
);

// 发送
send(sockfd, frame_buf, len);
```

---

## 🚀 快速开始

### 1. 初始化协议栈

```c
#include "proto_stack.h"

// 配置
proto_config_t cfg = PROTO_CONFIG_DEFAULT;
cfg.local_addr = 0x01;
cfg.rx_timeout_ms = 1000;
cfg.tx_retry = 3;

// 初始化
proto_stack_t stack;
proto_stack_init(&stack, &cfg, 
                 uart_send_fn, uart_recv_fn, uart_handle);

// 设置回调
proto_stack_set_callback(&stack,
    on_frame_received,    // 收到帧回调
    on_session_timeout,  // 超时回调
    on_error,             // 错误回调
    user_arg
);
```

### 2. 发送数据

```c
// 发送读命令
uint8_t reg_addr[] = { 0x00, 0x10 };
proto_stack_send(&stack, 0x02, PROTO_CMD_READ, 
                 reg_addr, sizeof(reg_addr));

// 发送写命令
uint8_t reg_value[] = { 0x00, 0x00, 0x12, 0x34 };
proto_stack_send(&stack, 0x02, PROTO_CMD_WRITE,
                 reg_value, sizeof(reg_value));

// 发送心跳
proto_stack_send_heartbeat(&stack, 0xFF);
```

### 3. 主循环轮询

```c
while (1) {
    // 接收数据
    proto_stack_poll(&stack, now_ms);
    
    // 处理会话（心跳、超时、重试）
    proto_stack_process(&stack, now_ms);
    
    usleep(10000);  // 10ms
}
```

---

## 📋 命令码定义

| 命令码   | 值   | 说明           |
|----------|------|----------------|
| READ     | 0x01 | 读数据         |
| WRITE    | 0x02 | 写数据         |
| REPORT   | 0x03 | 主动上报       |
| HEARTBEAT| 0x04 | 心跳           |
| ACK      | 0x05 | 应答确认       |
| NACK     | 0x06 | 应答否定       |
| RESET    | 0x07 | 复位           |
| UPDATE   | 0x08 | 固件升级       |
| ERROR    | 0xFF | 错误响应       |

---

## 🔌 适配不同通道

同一套协议栈，无缝切换物理通道：

### UART (串口)

```c
uart_handle_t *uart = uart_open("/dev/ttyUSB0", &(uart_config_t){
    .baudrate = 115200,
    .databits = 8,
    .stopbits = 1,
    .parity = 0,
});

proto_stack_init(&stack, &cfg,
    uart_transport_send, uart_transport_recv, uart);
```

### TCP 客户端

```c
socket_handle_t *sock = socket_create(&(socket_config_t){
    .type = SOCKET_TYPE_TCP_CLIENT,
    .remote_ip = "192.168.1.100",
    .remote_port = 8888,
});

proto_stack_init(&stack, &cfg,
    socket_transport_send, socket_transport_recv, sock);
```

### TCP 服务端

```c
socket_handle_t *server = socket_create(&(socket_config_t){
    .type = SOCKET_TYPE_TCP_SERVER,
    .local_port = 8888,
});

// 接受客户端连接
socket_handle_t *client = socket_accept(server);
```

### UDP

```c
socket_handle_t *sock = socket_create(&(socket_config_t){
    .type = SOCKET_TYPE_UDP,
    .remote_ip = "192.168.1.100",
    .remote_port = 8888,
    .local_port = 8889,
});
```

---

## 🔒 会话管理

协议栈内置会话管理，处理心跳、超时、重连：

```c
// 检查会话状态
session_state_t state = proto_stack_session_state(&stack);

// 会话自动处理：
// - 超时重传（默认3次，间隔500ms）
// - 等待ACK（超时判定设备离线）
// - 心跳保活（可配置间隔）
// - 断线重连（需用户实现回调）
```

---

## 🧪 测试帧示例

发送（十六进制）：
```
AA BB 00 09 02 01 00 10 7A 85 CC DD
 │  │  ───  ─  ─  ───  ───  ───  │
 │  │   │    │  │    │     │    帧尾
 │  │   │    │  │    │    CRC16
 │  │   │    │  │   数据
 │  │   │    │  命令码(读)
 │  │   │    地址(从机2)
 │  │  长度(9字节)
 帧头
```

解析后：
- 地址：0x02
- 命令：0x01 (READ)
- 数据：0x00 0x10
- CRC：0x7A85

---

## 📝 编译

```bash
# Linux
gcc -o example example/example_uart.c \
    src/core/*.c src/proto/*.c src/transport/*.c

# 或使用Makefile
make
```

---

## ⚠️ 注意事项

1. **环形缓冲区大小**：根据最大帧长配置，建议 `最大帧长 × 2`
2. **CRC计算范围**：从长度到数据结束，不包含CRC本身
3. **超时配置**：根据物理通道延迟调整，串口建议1000ms，TCP建议500ms
4. **多线程安全**：协议栈本身非线程安全，多线程环境需加锁保护

---

## 📜 License

MIT License

---

*本协议栈用于教育学习，实际工业应用请根据需求完善错误处理和边界检查。*
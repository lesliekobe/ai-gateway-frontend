/**
 * @file transport_uart.h
 * @brief UART传输层适配器
 *
 * 适配串口RS232/RS485通信
 */
#ifndef TRANSPORT_UART_H
#define TRANSPORT_UART_H

#include <stdint.h>
#include <stdbool.h>

/* =============== 前向声明 =============== */
typedef struct uart_handle uart_handle_t;

/* =============== UART配置 =============== */
typedef struct {
    uint32_t baudrate;     /* 波特率：9600/115200等 */
    uint8_t  databits;     /* 数据位：7/8 */
    uint8_t  stopbits;     /* 停止位：1/2 */
    uint8_t  parity;       /* 校验：0无，1奇，2偶 */
    uint16_t rx_timeout_ms; /* 接收超时 */
} uart_config_t;

/* =============== UART传输层接口 =============== */
/**
 * 打开串口
 * @param port  串口名，如"COM1"、"/dev/ttyS0"
 * @param config 串口配置
 * @return 串口句柄，NULL失败
 */
uart_handle_t* uart_open(const char *port, const uart_config_t *config);

/**
 * 关闭串口
 */
void uart_close(uart_handle_t *handle);

/**
 * 发送数据
 * @return 实际发送字节数，-1失败
 */
int uart_send(uart_handle_t *handle, const uint8_t *data, uint16_t len);

/**
 * 接收数据（非阻塞）
 * @param buf     接收缓冲区
 * @param buf_size 缓冲区大小
 * @return 实际读取字节数，-1失败，0无数据
 */
int uart_recv(uart_handle_t *handle, uint8_t *buf, uint16_t buf_size);

/**
 * 获取串口状态
 */
bool uart_is_open(uart_handle_t *handle);

/* =============== 统一传输接口适配 =============== */
/**
 * UART发送函数适配（用于proto_stack）
 */
int uart_transport_send(const uint8_t *data, uint16_t len, void *arg);

/**
 * UART接收函数适配（用于proto_stack）
 */
int uart_transport_recv(uint8_t *buf, uint16_t buf_size, void *arg);

#endif /* TRANSPORT_UART_H */
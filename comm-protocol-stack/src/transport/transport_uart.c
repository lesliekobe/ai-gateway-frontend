/**
 * @file transport_uart.c
 * @brief UART传输层实现（桩实现）
 *
 * 具体实现需根据目标平台（STM32/ESP32/Linux）编写
 * 这里提供接口框架和模拟实现
 */
#include "transport_uart.h"
#include <stdio.h>
#include <string.h>

/* =============== 桩实现说明 =============== */
/*
 * 以下为跨平台的桩实现，实际使用时需要：
 * - Linux: 使用termios + file descriptor
 * - STM32: 使用HAL_UART_XXX
 * - ESP32: 使用uart-driver
 */

/**
 * 模拟的UART发送（实际替换为平台实现）
 */
int uart_send(uart_handle_t *handle, const uint8_t *data, uint16_t len) {
    if (!handle || !data || len == 0)
        return -1;
    
    /* TODO: 调用实际UART发送接口 */
    /* 示例：HAL_UART_Transmit(&huart, data, len, timeout); */
    
    return len;
}

/**
 * 模拟的UART接收（实际替换为平台实现）
 */
int uart_recv(uart_handle_t *handle, uint8_t *buf, uint16_t buf_size) {
    if (!handle || !buf || buf_size == 0)
        return -1;
    
    /* TODO: 调用实际UART接收接口 */
    /* 示例：HAL_UART_Receive(&huart, buf, expected_len, timeout); */
    
    return 0;  /* 返回0表示无数据，需上层轮询 */
}

/**
 * UART发送适配器（用于proto_stack）
 */
int uart_transport_send(const uint8_t *data, uint16_t len, void *arg) {
    uart_handle_t *handle = (uart_handle_t *)arg;
    return uart_send(handle, data, len);
}

/**
 * UART接收适配器（用于proto_stack）
 */
int uart_transport_recv(uint8_t *buf, uint16_t buf_size, void *arg) {
    uart_handle_t *handle = (uart_handle_t *)arg;
    return uart_recv(handle, buf, buf_size);
}

/* =============== 桩：实际实现需要替换 =============== */
/*
 * Linux实现示例：
 * 
 * int uart_open(const char *port, const uart_config_t *config) {
 *     int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
 *     if (fd < 0) return NULL;
 *     
 *     struct termios tty;
 *     cfsetospeed(&tty, B115200);
 *     // ... 配置校验位、停止位等
 *     
 *     return fd;  // 返回文件描述符作为句柄
 * }
 */
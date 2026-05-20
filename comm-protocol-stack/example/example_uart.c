/**
 * @file example_uart.c
 * @brief UART协议通信示例
 *
 * 演示如何使用协议栈通过串口收发数据
 */
#include "proto_stack.h"
#include "transport_uart.h"
#include <stdio.h>
#include <unistd.h>

/* =============== 全局协议栈实例 =============== */
static proto_stack_t g_proto_stack;

/* =============== 收到帧的回调 =============== */
void on_frame_received(const proto_frame_t *frame, void *arg) {
    printf("\n[RECV] addr=0x%02X cmd=0x%02X len=%u\n", 
           frame->addr, frame->cmd, frame->data_len);
    
    /* 打印数据域 */
    printf("       data: ");
    for (uint16_t i = 0; i < frame->data_len; i++) {
        printf("%02X ", frame->data[i]);
    }
    printf("\n");
    
    /* 发送ACK确认 */
    proto_stack_send_ack(&g_proto_stack, frame->addr, frame->cmd);
}

/* =============== 会话超时的回调 =============== */
void on_session_timeout(uint8_t addr, void *arg) {
    printf("\n[WARN] Session timeout, addr=0x%02X\n", addr);
}

/* =============== 错误回调 =============== */
void on_error(proto_err_t err, uint8_t addr, void *arg) {
    printf("\n[ERROR] err=%d addr=0x%02X\n", err, addr);
}

/* =============== 主函数 =============== */
int main(int argc, char *argv[]) {
    printf("=== UART Protocol Stack Demo ===\n");
    
    /* 1. 打开串口 */
    uart_config_t uart_cfg = {
        .baudrate = 115200,
        .databits = 8,
        .stopbits = 1,
        .parity = 0,
        .rx_timeout_ms = 1000,
    };
    
    uart_handle_t *uart = uart_open("/dev/ttyUSB0", &uart_cfg);
    if (!uart) {
        printf("[ERROR] UART open failed\n");
        return -1;
    }
    printf("[OK] UART opened\n");
    
    /* 2. 初始化协议栈 */
    proto_config_t proto_cfg = PROTO_CONFIG_DEFAULT;
    proto_cfg.local_addr = 0x01;
    proto_cfg.rx_timeout_ms = 1000;
    proto_cfg.tx_retry = 3;
    proto_cfg.tx_retry_ms = 500;
    
    if (proto_stack_init(&g_proto_stack, &proto_cfg,
                         uart_transport_send, uart_transport_recv, uart) != 0) {
        printf("[ERROR] Protocol stack init failed\n");
        uart_close(uart);
        return -1;
    }
    
    proto_stack_set_callback(&g_proto_stack, on_frame_received, 
                            on_session_timeout, on_error, NULL);
    printf("[OK] Protocol stack initialized\n");
    
    /* 3. 主循环 */
    printf("\n--- Entering main loop, press Ctrl+C to exit ---\n");
    
    while (1) {
        /* 轮询接收数据 */
        proto_stack_poll(&g_proto_stack, 0);
        
        /* 处理会话状态（心跳、超时） */
        proto_stack_process(&g_proto_stack, 0);
        
        /* 每5秒发送一次心跳 */
        static uint32_t last_hb = 0;
        uint32_t now = 0;  /* 实际使用时用真实时间戳 */
        if (now - last_hb > 5000) {
            proto_stack_send_heartbeat(&g_proto_stack, 0xFF);  /* 广播 */
            last_hb = now;
        }
        
        usleep(10000);  /* 10ms间隔 */
    }
    
    /* 4. 清理 */
    proto_stack_deinit(&g_proto_stack);
    uart_close(uart);
    
    return 0;
}
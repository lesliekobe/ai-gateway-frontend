/**
 * @file example_tcp.c
 * @brief TCP协议通信示例
 *
 * 演示如何使用协议栈通过TCP收发数据
 */
#include "proto_stack.h"
#include "transport_socket.h"
#include <stdio.h>
#include <unistd.h>

/* =============== 全局协议栈实例 =============== */
static proto_stack_t g_proto_stack;
static socket_handle_t *g_socket = NULL;

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
    printf("\n[WARN] Session timeout, addr=0x%02X, reconnecting...\n", addr);
    
    /* 尝试重新连接 */
    socket_destroy(g_socket);
    g_socket = socket_create(&(socket_config_t){
        .type = SOCKET_TYPE_TCP_CLIENT,
        .remote_ip = "192.168.1.100",
        .remote_port = 8888,
        .recv_timeout_ms = 1000,
    });
}

/* =============== 错误回调 =============== */
void on_error(proto_err_t err, uint8_t addr, void *arg) {
    printf("\n[ERROR] err=%d addr=0x%02X\n", err, addr);
}

/* =============== 主函数 =============== */
int main(int argc, char *argv[]) {
    printf("=== TCP Protocol Stack Demo ===\n");
    
    /* 1. 创建TCP连接 */
    socket_config_t sock_cfg = {
        .type = SOCKET_TYPE_TCP_CLIENT,
        .remote_ip = "192.168.1.100",
        .remote_port = 8888,
        .recv_timeout_ms = 1000,
    };
    
    g_socket = socket_create(&sock_cfg);
    if (!g_socket) {
        printf("[ERROR] Socket create failed\n");
        return -1;
    }
    printf("[OK] TCP connected to %s:%d\n", sock_cfg.remote_ip, sock_cfg.remote_port);
    
    /* 2. 初始化协议栈 */
    proto_config_t proto_cfg = PROTO_CONFIG_DEFAULT;
    proto_cfg.local_addr = 0x01;
    proto_cfg.rx_timeout_ms = 1000;
    proto_cfg.tx_retry = 3;
    proto_cfg.tx_retry_ms = 500;
    
    if (proto_stack_init(&g_proto_stack, &proto_cfg,
                         socket_transport_send, socket_transport_recv, g_socket) != 0) {
        printf("[ERROR] Protocol stack init failed\n");
        socket_destroy(g_socket);
        return -1;
    }
    
    proto_stack_set_callback(&g_proto_stack, on_frame_received, 
                            on_session_timeout, on_error, NULL);
    printf("[OK] Protocol stack initialized\n");
    
    /* 3. 演示：发送读寄存器命令 */
    uint8_t read_reg_cmd[] = { 0x00, 0x10 };  /* 读寄存器0x0010 */
    proto_stack_send(&g_proto_stack, 0x02, PROTO_CMD_READ, read_reg_cmd, sizeof(read_reg_cmd));
    printf("\n[SEND] Read register 0x0010\n");
    
    /* 4. 主循环 */
    printf("\n--- Entering main loop, press Ctrl+C to exit ---\n");
    
    while (1) {
        /* 检查连接状态 */
        if (!socket_is_connected(g_socket)) {
            printf("[WARN] Connection lost, reconnecting...\n");
            sleep(1);
            g_socket = socket_create(&sock_cfg);
            continue;
        }
        
        /* 轮询接收数据 */
        proto_stack_poll(&g_proto_stack, 0);
        
        /* 处理会话状态（心跳、超时） */
        proto_stack_process(&g_proto_stack, 0);
        
        usleep(10000);  /* 10ms间隔 */
    }
    
    /* 5. 清理 */
    proto_stack_deinit(&g_proto_stack);
    socket_destroy(g_socket);
    
    return 0;
}
/**
 * @file transport_socket.c
 * @brief Socket传输层实现（桩实现）
 *
 * 具体实现需根据目标平台编写
 */
#include "transport_socket.h"
#include <stdio.h>
#include <string.h>

/* =============== 桩实现说明 =============== */
/*
 * 以下为跨平台的桩实现，实际使用时需要：
 * - Linux: 使用 BSD socket API
 * - Windows: 使用 Winsock2
 * - ESP32: 使用 lwIP socket API
 */

/**
 * 模拟的Socket发送（实际替换为平台实现）
 */
int socket_send(socket_handle_t *handle, const uint8_t *data, uint16_t len) {
    if (!handle || !data || len == 0)
        return -1;
    
    /* TODO: 调用实际Socket发送接口 */
    /* 示例：send(sockfd, data, len, 0); */
    
    return len;
}

/**
 * 模拟的Socket接收（实际替换为平台实现）
 */
int socket_recv(socket_handle_t *handle, uint8_t *buf, uint16_t buf_size) {
    if (!handle || !buf || buf_size == 0)
        return -1;
    
    /* TODO: 调用实际Socket接收接口 */
    /* 示例：recv(sockfd, buf, buf_size, 0); */
    
    return 0;  /* 返回0表示无数据，需上层轮询 */
}

/**
 * Socket发送适配器（用于proto_stack）
 */
int socket_transport_send(const uint8_t *data, uint16_t len, void *arg) {
    socket_handle_t *handle = (socket_handle_t *)arg;
    return socket_send(handle, data, len);
}

/**
 * Socket接收适配器（用于proto_stack）
 */
int socket_transport_recv(uint8_t *buf, uint16_t buf_size, void *arg) {
    socket_handle_t *handle = (socket_handle_t *)arg;
    return socket_recv(handle, buf, buf_size);
}

/* =============== 桩：实际实现需要替换 =============== */
/*
 * Linux TCP客户端示例：
 * 
 * socket_handle_t* socket_create(const socket_config_t *config) {
 *     int sock = socket(AF_INET, SOCK_STREAM, 0);
 *     if (sock < 0) return NULL;
 *     
 *     struct sockaddr_in addr;
 *     addr.sin_family = AF_INET;
 *     addr.sin_port = htons(config->remote_port);
 *     inet_pton(AF_INET, config->remote_ip, &addr.sin_addr);
 *     
 *     if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
 *         close(sock);
 *         return NULL;
 *     }
 *     
 *     return (socket_handle_t *)sock;
 * }
 */
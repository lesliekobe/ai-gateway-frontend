/**
 * @file transport_socket.h
 * @brief Socket传输层适配器
 *
 * 适配TCP客户端/服务端、UDP通信
 */
#ifndef TRANSPORT_SOCKET_H
#define TRANSPORT_SOCKET_H

#include <stdint.h>
#include <stdbool.h>

/* =============== Socket类型 =============== */
typedef enum {
    SOCKET_TYPE_TCP_CLIENT = 0,  /* TCP客户端 */
    SOCKET_TYPE_TCP_SERVER,      /* TCP服务端 */
    SOCKET_TYPE_UDP,             /* UDP */
} socket_type_t;

/* =============== Socket配置 =============== */
typedef struct {
    socket_type_t type;         /* Socket类型 */
    const char   *remote_ip;     /* 远端IP（TCP客户端/UDP需要） */
    uint16_t      remote_port;   /* 远端端口 */
    uint16_t      local_port;    /* 本地端口（TCP服务端/UDP需要） */
    uint16_t      recv_timeout_ms; /* 接收超时 */
} socket_config_t;

/* =============== Socket句柄 =============== */
typedef struct socket_handle socket_handle_t;

/**
 * 创建Socket连接
 * @param config Socket配置
 * @return Socket句柄，NULL失败
 */
socket_handle_t* socket_create(const socket_config_t *config);

/**
 * 销毁Socket连接
 */
void socket_destroy(socket_handle_t *handle);

/**
 * 发送数据
 * @return 实际发送字节数，-1失败
 */
int socket_send(socket_handle_t *handle, const uint8_t *data, uint16_t len);

/**
 * 接收数据（非阻塞）
 * @return 实际读取字节数，-1失败，0无数据
 */
int socket_recv(socket_handle_t *handle, uint8_t *buf, uint16_t buf_size);

/**
 * 检查是否连接
 */
bool socket_is_connected(socket_handle_t *handle);

/**
 * 主动断开连接
 */
void socket_disconnect(socket_handle_t *handle);

/**
 * TCP服务端：接受客户端连接
 * @return 客户端Socket句柄，NULL无连接
 */
socket_handle_t* socket_accept(socket_handle_t *server);

/* =============== 统一传输接口适配 =============== */
int socket_transport_send(const uint8_t *data, uint16_t len, void *arg);
int socket_transport_recv(uint8_t *buf, uint16_t buf_size, void *arg);

#endif /* TRANSPORT_SOCKET_H */
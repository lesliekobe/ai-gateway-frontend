/**
 * @file proto_stack.h
 * @brief 协议栈核心接口
 *
 * 统一接口层：应用层通过这些接口发帧收帧，不用管底层是串口还是TCP
 */
#ifndef PROTO_STACK_H
#define PROTO_STACK_H

#include "../core/ringbuffer.h"
#include "proto_frame.h"
#include "proto_parser.h"
#include "proto_packer.h"
#include "session.h"

/* =============== 传输层接口 =============== */
/**
 * 传输层发送函数类型
 * 适配不同底层：串口、TCP、CAN等
 */
typedef int (*transport_send_t)(const uint8_t *data, uint16_t len, void *arg);

/**
 * 传输层接收函数类型
 * 返回实际读取的字节数
 */
typedef int (*transport_recv_t)(uint8_t *buf, uint16_t buf_size, void *arg);

/* =============== 协议栈实例 =============== */
typedef struct {
    /* 配置 */
    proto_config_t config;
    
    /* 接收缓冲区 */
    ringbuffer_t rx_rb;
    uint8_t rx_buffer[2048];  /* 接收环缓冲 */
    
    /* 解析器 */
    proto_parser_t parser;
    
    /* 会话管理 */
    session_t session;
    
    /* 传输层接口 */
    transport_send_t send_fn;
    transport_recv_t recv_fn;
    void *transport_arg;
    
    /* 用户回调 */
    void (*on_frame_recv)(const proto_frame_t *frame, void *arg);   /* 收到完整帧 */
    void (*on_session_timeout)(uint8_t addr, void *arg);           /* 会话超时 */
    void (*on_error)(proto_err_t err, uint8_t addr, void *arg);    /* 错误回调 */
    void *user_arg;
} proto_stack_t;

/**
 * 初始化协议栈
 * @param stack       协议栈句柄
 * @param config      配置参数
 * @param send_fn     发送回调
 * @param recv_fn     接收回调
 * @param transport_arg 传输层参数
 */
int proto_stack_init(proto_stack_t *stack, const proto_config_t *config,
                     transport_send_t send_fn, transport_recv_t recv_fn, void *transport_arg);

/**
 * 销毁协议栈
 */
void proto_stack_deinit(proto_stack_t *stack);

/**
 * 设置回调函数
 */
void proto_stack_set_callback(proto_stack_t *stack,
                              void (*on_frame_recv)(const proto_frame_t *, void *),
                              void (*on_session_timeout)(uint8_t, void *),
                              void (*on_error)(proto_err_t, uint8_t, void *),
                              void *user_arg);

/**
 * 发送协议帧
 * @return 发送的帧长度，0失败
 */
uint16_t proto_stack_send(proto_stack_t *stack, uint8_t addr, uint8_t cmd, 
                           const uint8_t *data, uint16_t len);

/**
 * 发送原始帧（已组帧的数据）
 */
int proto_stack_send_raw(proto_stack_t *stack, const uint8_t *frame, uint16_t len);

/**
 * 轮询接收数据（需要在循环中调用，或定时器触发）
 * @return 0有收到帧，-1无数据，-2错误
 */
int proto_stack_poll(proto_stack_t *stack, uint32_t now_ms);

/**
 * 处理会话状态（心跳、超时、重试）
 * @return 0正常，1需要重发，2会话断开
 */
int proto_stack_process(proto_stack_t *stack, uint32_t now_ms);

/**
 * 发送心跳帧
 */
int proto_stack_send_heartbeat(proto_stack_t *stack, uint8_t addr);

/**
 * 发送ACK确认
 */
int proto_stack_send_ack(proto_stack_t *stack, uint8_t addr, uint8_t cmd);

/**
 * 获取当前会话状态
 */
session_state_t proto_stack_session_state(proto_stack_t *stack);

#endif /* PROTO_STACK_H */
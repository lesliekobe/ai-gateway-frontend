/**
 * @file session.h
 * @brief 通信会话管理 - 心跳、ACK、超时重传、断线重连
 */
#ifndef SESSION_H
#define SESSION_H

#include "proto_frame.h"
#include <stdbool.h>

/* =============== 会话状态 =============== */
typedef enum {
    SESSION_STATE_IDLE = 0,       /* 空闲 */
    SESSION_STATE_CONNECTED,     /* 已连接 */
    SESSION_STATE_BUSY,          /* 等待应答 */
    SESSION_STATE_TIMEOUT,       /* 超时 */
    SESSION_STATE_ERROR,         /* 错误 */
} session_state_t;

/* =============== 会话结构 =============== */
typedef struct {
    uint8_t  remote_addr;           /* 远端设备地址 */
    session_state_t state;          /* 当前状态 */
    uint32_t last_send_time;        /* 上次发送时间（ms） */
    uint32_t last_recv_time;        /* 上次接收时间（ms） */
    uint16_t retry_count;          /* 当前重试次数 */
    uint16_t max_retry;            /* 最大重试次数 */
    uint16_t timeout_ms;           /* 超时时间（ms） */
    uint16_t heartbeat_interval_ms; /* 心跳间隔（ms） */
    bool    waiting_ack;           /* 是否等待ACK */
} session_t;

/**
 * 初始化会话
 * @param session 会话句柄
 * @param addr    远端设备地址
 * @param config  超时和重试参数
 */
void session_init(session_t *session, uint8_t addr, uint16_t timeout_ms, uint16_t max_retry, uint16_t heartbeat_ms);

/**
 * 重置会话
 */
void session_reset(session_t *session);

/**
 * 标记会话已发送数据（启动超时计时）
 */
void session_on_send(session_t *session, uint32_t now_ms);

/**
 * 标记会话收到数据（更新接收时间，清除等待ACK）
 */
void session_on_recv(session_t *session, uint32_t now_ms);

/**
 * 检查会话是否超时
 * @return true 超时
 */
bool session_is_timeout(session_t *session, uint32_t now_ms);

/**
 * 检查是否需要重试
 * @return true 需要重试
 */
bool session_should_retry(session_t *session, uint32_t now_ms);

/**
 * 检查是否需要发送心跳
 * @return true 需要发送心跳
 */
bool session_should_heartbeat(session_t *session, uint32_t now_ms);

/**
 * 获取会话状态描述
 */
const char* session_state_str(session_state_t state);

#endif /* SESSION_H */
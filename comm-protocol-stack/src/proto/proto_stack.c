/**
 * @file proto_stack.c
 * @brief 协议栈核心实现
 */
#include "proto_stack.h"
#include <string.h>

int proto_stack_init(proto_stack_t *stack, const proto_config_t *config,
                     transport_send_t send_fn, transport_recv_t recv_fn, void *transport_arg) {
    if (!stack || !config || !send_fn || !recv_fn)
        return -1;
    
    /* 初始化配置 */
    memcpy(&stack->config, config, sizeof(proto_config_t));
    
    /* 初始化接收环形缓冲区 */
    if (ringbuffer_init(&stack->rx_rb, stack->rx_buffer, sizeof(stack->rx_buffer)) != 0)
        return -1;
    
    /* 初始化解析器 */
    proto_parser_init(&stack->parser);
    
    /* 初始化会话 */
    session_init(&stack->session, 0, config->rx_timeout_ms, 
                 config->tx_retry, config->tx_retry_ms);
    
    /* 传输层接口 */
    stack->send_fn = send_fn;
    stack->recv_fn = recv_fn;
    stack->transport_arg = transport_arg;
    
    /* 默认回调 */
    stack->on_frame_recv = NULL;
    stack->on_session_timeout = NULL;
    stack->on_error = NULL;
    stack->user_arg = NULL;
    
    return 0;
}

void proto_stack_deinit(proto_stack_t *stack) {
    if (!stack)
        return;
    
    ringbuffer_deinit(&stack->rx_rb);
    proto_parser_reset(&stack->parser);
    session_reset(&stack->session);
}

void proto_stack_set_callback(proto_stack_t *stack,
                              void (*on_frame_recv)(const proto_frame_t *, void *),
                              void (*on_session_timeout)(uint8_t, void *),
                              void (*on_error)(proto_err_t, uint8_t, void *),
                              void *user_arg) {
    if (!stack)
        return;
    
    stack->on_frame_recv = on_frame_recv;
    stack->on_session_timeout = on_session_timeout;
    stack->on_error = on_error;
    stack->user_arg = user_arg;
}

uint16_t proto_stack_send(proto_stack_t *stack, uint8_t addr, uint8_t cmd, 
                          const uint8_t *data, uint16_t len) {
    if (!stack)
        return 0;
    
    uint8_t frame[PROTO_MAX_FRAME_SIZE];
    uint16_t frame_len = proto_pack(addr, cmd, data, len, frame, sizeof(frame));
    
    if (frame_len == 0)
        return 0;
    
    int ret = stack->send_fn(frame, frame_len, stack->transport_arg);
    if (ret > 0) {
        /* 记录发送时间，用于超时判断 */
        session_on_send(&stack->session, 0);  /* 0表示使用当前时间，但这里由调用者传入 */
        return frame_len;
    }
    
    return 0;
}

int proto_stack_send_raw(proto_stack_t *stack, const uint8_t *frame, uint16_t len) {
    if (!stack || !frame || len == 0)
        return -1;
    
    return stack->send_fn(frame, len, stack->transport_arg);
}

int proto_stack_poll(proto_stack_t *stack, uint32_t now_ms) {
    if (!stack)
        return -2;
    
    /* 从底层读取数据到环形缓冲区 */
    uint8_t tmp[256];
    int read = stack->recv_fn(tmp, sizeof(tmp), stack->transport_arg);
    
    if (read > 0) {
        ringbuffer_write(&stack->rx_rb, tmp, read);
        
        /* 解析环形缓冲区中的帧 */
        proto_frame_t frame;
        int ret = proto_parser_parse_rb(&stack->parser, &stack->rx_rb, &frame);
        
        if (ret == 0) {
            /* 成功收到完整帧 */
            session_on_recv(&stack->session, now_ms);
            
            /* 调用用户回调 */
            if (stack->on_frame_recv) {
                stack->on_frame_recv(&frame, stack->user_arg);
            }
            
            return 0;
        }
    }
    
    return -1;  /* 无数据 */
}

int proto_stack_process(proto_stack_t *stack, uint32_t now_ms) {
    if (!stack)
        return -2;
    
    /* 检查会话超时 */
    if (session_is_timeout(&stack->session, now_ms)) {
        if (session_should_retry(&stack->session, now_ms)) {
            return 1;  /* 需要重发 */
        } else {
            /* 超时次数用尽，通知用户 */
            if (stack->on_session_timeout) {
                stack->on_session_timeout(stack->session.remote_addr, stack->user_arg);
            }
            session_reset(&stack->session);
            return 2;  /* 会话断开 */
        }
    }
    
    return 0;
}

int proto_stack_send_heartbeat(proto_stack_t *stack, uint8_t addr) {
    if (!stack)
        return -1;
    
    uint8_t frame[16];
    uint16_t len = proto_pack(addr, PROTO_CMD_HEARTBEAT, NULL, 0, frame, sizeof(frame));
    
    if (len == 0)
        return -1;
    
    return stack->send_fn(frame, len, stack->transport_arg);
}

int proto_stack_send_ack(proto_stack_t *stack, uint8_t addr, uint8_t cmd) {
    if (!stack)
        return -1;
    
    uint8_t data[2] = { cmd, PROTO_ERR_NONE };
    uint8_t frame[16];
    uint16_t len = proto_pack(addr, PROTO_CMD_ACK, data, 2, frame, sizeof(frame));
    
    if (len == 0)
        return -1;
    
    return stack->send_fn(frame, len, stack->transport_arg);
}

session_state_t proto_stack_session_state(proto_stack_t *stack) {
    if (!stack)
        return SESSION_STATE_ERROR;
    
    return stack->session.state;
}
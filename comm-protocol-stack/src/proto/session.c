/**
 * @file session.c
 * @brief 通信会话管理实现
 */
#include "session.h"

void session_init(session_t *session, uint8_t addr, uint16_t timeout_ms, 
                  uint16_t max_retry, uint16_t heartbeat_ms) {
    if (!session)
        return;
    
    session->remote_addr = addr;
    session->state = SESSION_STATE_IDLE;
    session->last_send_time = 0;
    session->last_recv_time = 0;
    session->retry_count = 0;
    session->max_retry = max_retry;
    session->timeout_ms = timeout_ms;
    session->heartbeat_interval_ms = heartbeat_ms;
    session->waiting_ack = false;
}

void session_reset(session_t *session) {
    if (!session)
        return;
    
    session->state = SESSION_STATE_IDLE;
    session->retry_count = 0;
    session->waiting_ack = false;
}

void session_on_send(session_t *session, uint32_t now_ms) {
    if (!session)
        return;
    
    session->last_send_time = now_ms;
    session->waiting_ack = true;
    session->state = SESSION_STATE_BUSY;
}

void session_on_recv(session_t *session, uint32_t now_ms) {
    if (!session)
        return;
    
    session->last_recv_time = now_ms;
    session->waiting_ack = false;
    session->retry_count = 0;
    session->state = SESSION_STATE_CONNECTED;
}

bool session_is_timeout(session_t *session, uint32_t now_ms) {
    if (!session)
        return false;
    
    if (!session->waiting_ack)
        return false;
    
    return (now_ms - session->last_send_time) > session->timeout_ms;
}

bool session_should_retry(session_t *session, uint32_t now_ms) {
    if (!session)
        return false;
    
    if (!session->waiting_ack)
        return false;
    
    if (session->retry_count >= session->max_retry)
        return false;
    
    if (!session_is_timeout(session, now_ms))
        return false;
    
    return true;
}

bool session_should_heartbeat(session_t *session, uint32_t now_ms) {
    if (!session)
        return false;
    
    if (session->waiting_ack)
        return false;
    
    return (now_ms - session->last_recv_time) > session->heartbeat_interval_ms;
}

const char* session_state_str(session_state_t state) {
    switch (state) {
        case SESSION_STATE_IDLE:      return "IDLE";
        case SESSION_STATE_CONNECTED:  return "CONNECTED";
        case SESSION_STATE_BUSY:      return "BUSY";
        case SESSION_STATE_TIMEOUT:   return "TIMEOUT";
        case SESSION_STATE_ERROR:     return "ERROR";
        default: return "UNKNOWN";
    }
}
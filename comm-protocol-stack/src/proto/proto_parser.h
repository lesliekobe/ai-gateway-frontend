/**
 * @file proto_parser.h
 * @brief 协议解析器 - 解包状态机实现
 *
 * 接收端是字节流，不是整帧，必须用状态机逐字节解析：
 *
 * 状态流转：
 * WAIT_HEAD1 → WAIT_HEAD2 → WAIT_LEN1 → WAIT_LEN2 → 
 * WAIT_ADDR → WAIT_CMD → WAIT_DATA → WAIT_CRC1 → WAIT_CRC2 → WAIT_TAIL1 → WAIT_TAIL2 → COMPLETE
 *
 * 自动解决：粘包、半包、干扰乱码、错位
 */
#ifndef PROTO_PARSER_H
#define PROTO_PARSER_H

#include "proto_frame.h"
#include "../core/ringbuffer.h"

/* =============== 解包状态机状态 =============== */
typedef enum {
    PARSER_STATE_WAIT_HEAD1 = 0,    /* 等待帧头第1字节 (0xAA) */
    PARSER_STATE_WAIT_HEAD2,       /* 等待帧头第2字节 (0xBB) */
    PARSER_STATE_WAIT_LEN1,        /* 等待长度高字节 */
    PARSER_STATE_WAIT_LEN2,        /* 等待长度低字节 */
    PARSER_STATE_WAIT_ADDR,        /* 等待设备地址 */
    PARSER_STATE_WAIT_CMD,         /* 等待命令码 */
    PARSER_STATE_WAIT_DATA,        /* 等待数据域 */
    PARSER_STATE_WAIT_CRC1,        /* 等待CRC高字节 */
    PARSER_STATE_WAIT_CRC2,        /* 等待CRC低字节 */
    PARSER_STATE_WAIT_TAIL1,       /* 等待帧尾第1字节 (0xCC) */
    PARSER_STATE_WAIT_TAIL2,       /* 等待帧尾第2字节 (0xDD) */
    PARSER_STATE_COMPLETE,        /* 完整帧已解析 */
    PARSER_STATE_ERROR,            /* 解析错误 */
} parser_state_t;

/* =============== 解析器结构 =============== */
typedef struct {
    parser_state_t state;          /* 当前状态 */
    uint16_t expected_len;        /* 期望的数据长度 */
    uint16_t received_len;        /* 已接收的数据长度 */
    proto_frame_t frame;          /* 解析后的完整帧 */
    uint16_t crc_calc;            /* 计算出的CRC */
    uint8_t  error;              /* 错误码 */
} proto_parser_t;

/**
 * 初始化解析器
 */
void proto_parser_init(proto_parser_t *parser);

/**
 * 重置解析器到初始状态
 */
void proto_parser_reset(proto_parser_t *parser);

/**
 * 向解析器喂入一个字节，返回当前状态
 * @param parser 解析器句柄
 * @param byte   接收到的字节
 * @return 0成功解析到完整帧，-1解析中，-2解析错误
 */
int proto_parser_feed(proto_parser_t *parser, uint8_t byte);

/**
 * 从环形缓冲区解析帧
 * @param parser 解析器句柄
 * @param rb     环形缓冲区
 * @param frame  解析出的帧（输出）
 * @return 0成功，-1无完整帧，-2解析错误
 */
int proto_parser_parse_rb(proto_parser_t *parser, ringbuffer_t *rb, proto_frame_t *frame);

/**
 * 获取解析器当前状态描述
 */
const char* proto_parser_state_str(parser_state_t state);

#endif /* PROTO_PARSER_H */
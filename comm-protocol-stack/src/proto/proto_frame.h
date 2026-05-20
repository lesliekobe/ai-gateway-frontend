/**
 * @file proto_frame.h
 * @brief 协议帧格式定义
 *
 * 标准帧格式：
 * +--------+--------+--------+--------+--------+--------+--------+--------+
 * |  帧头  |  长度  | 地址   | 命令码 | 数据域(N字节)      | 校验   |  帧尾  |
 * | 2字节  | 2字节  | 1字节  | 1字节  | 0~65535字节        | 2字节  | 2字节  |
 * | AA BB  | LL HH  | 01-FF  | 01-FF  |                    | CRC16  | CC DD  |
 * +--------+--------+--------+--------+--------+--------+--------+--------+
 *
 * 说明：
 * - 帧头/帧尾：固定标识，用于帧同步
 * - 长度：整个帧不含头尾的长度（高字节在前，Big-Endian）
 * - 地址：设备从机地址 0x01-0xFF，0x00为广播地址
 * - 命令码：读、写、上报、心跳、复位等
 * - 数据域：实际业务数据，变长
 * - 校验：CRC16-Modbus
 */
#ifndef PROTO_FRAME_H
#define PROTO_FRAME_H

#include <stdint.h>

/* =============== 协议常量 =============== */
#define PROTO_HEAD          0xAA, 0xBB    /* 帧头 */
#define PROTO_TAIL          0xCC, 0xDD    /* 帧尾 */
#define PROTO_HEAD_SIZE     2
#define PROTO_TAIL_SIZE     2
#define PROTO_LEN_SIZE      2
#define PROTO_ADDR_SIZE     1
#define PROTO_CMD_SIZE      1
#define PROTO_CRC_SIZE      2
#define PROTO_MIN_SIZE      (PROTO_HEAD_SIZE + PROTO_LEN_SIZE + PROTO_ADDR_SIZE + PROTO_CMD_SIZE + PROTO_CRC_SIZE + PROTO_TAIL_SIZE)
#define PROTO_MAX_DATA_SIZE 1024          /* 最大数据域长度 */
#define PROTO_MAX_FRAME_SIZE (PROTO_MIN_SIZE + PROTO_MAX_DATA_SIZE)

/* =============== 地址定义 =============== */
#define PROTO_ADDR_BROADCAST  0x00    /* 广播地址 */
#define PROTO_ADDR_MIN        0x01    /* 最小从机地址 */
#define PROTO_ADDR_MAX        0xFF    /* 最大从机地址 */

/* =============== 命令码定义 =============== */
typedef enum {
    PROTO_CMD_READ       = 0x01,   /* 读数据 */
    PROTO_CMD_WRITE      = 0x02,   /* 写数据 */
    PROTO_CMD_REPORT     = 0x03,   /* 主动上报 */
    PROTO_CMD_HEARTBEAT  = 0x04,   /* 心跳 */
    PROTO_CMD_ACK        = 0x05,   /* 应答确认 */
    PROTO_CMD_NACK       = 0x06,   /* 应答否定 */
    PROTO_CMD_RESET      = 0x07,   /* 复位 */
    PROTO_CMD_UPDATE     = 0x08,   /* 固件升级 */
    PROTO_CMD_ERROR      = 0xFF,   /* 错误响应 */
} proto_cmd_t;

/* =============== 错误码定义 =============== */
typedef enum {
    PROTO_ERR_NONE       = 0x00,   /* 无错误 */
    PROTO_ERR_CRC        = 0x01,   /* 校验错误 */
    PROTO_ERR_LEN        = 0x02,   /* 长度错误 */
    PROTO_ERR_ADDR       = 0x03,   /* 地址错误 */
    PROTO_ERR_CMD        = 0x04,   /* 命令码错误 */
    PROTO_ERR_FRAME      = 0x05,   /* 帧格式错误 */
    PROTO_ERR_TIMEOUT    = 0x06,   /* 超时错误 */
    PROTO_ERR_OVERFLOW   = 0x07,   /* 缓冲区溢出 */
    PROTO_ERR_BUSY       = 0x08,   /* 设备忙 */
    PROTO_ERR_PARAM      = 0x09,   /* 参数错误 */
} proto_err_t;

/* =============== 协议帧结构 =============== */
/* 用于存储解析后的完整帧 */
typedef struct {
    uint8_t  addr;           /* 设备地址 */
    uint8_t  cmd;            /* 命令码 */
    uint16_t data_len;       /* 数据域长度 */
    uint8_t  data[PROTO_MAX_DATA_SIZE]; /* 数据域 */
    uint16_t crc;            /* CRC16校验码 */
} proto_frame_t;

/* =============== 协议栈配置 =============== */
typedef struct {
    uint8_t  local_addr;     /* 本机地址 */
    uint16_t max_frame_size; /* 最大帧长度 */
    uint16_t rx_timeout_ms;  /* 接收超时 ms */
    uint16_t tx_retry;       /* 发送重试次数 */
    uint16_t tx_retry_ms;    /* 重试间隔 ms */
} proto_config_t;

/* 默认配置 */
#define PROTO_CONFIG_DEFAULT { 0x01, PROTO_MAX_FRAME_SIZE, 1000, 3, 500 }

#endif /* PROTO_FRAME_H */
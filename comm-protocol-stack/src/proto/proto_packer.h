/**
 * @file proto_packer.h
 * @brief 协议封包器 - 发送端组帧
 *
 * 给定：命令码、地址、数据
 * 流程：填充帧头 → 填写长度 → 填地址/命令码 → 拷贝数据 → 计算校验 → 填充帧尾 → 发送
 */
#ifndef PROTO_PACKER_H
#define PROTO_PACKER_H

#include "proto_frame.h"
#include <stddef.h>

/**
 * 组装完整协议帧
 * @param addr      目标设备地址
 * @param cmd       命令码
 * @param data      业务数据指针
 * @param data_len  数据长度
 * @param frame_buf 输出帧缓冲区
 * @param buf_size  缓冲区大小
 * @return 实际组装的帧长度，0失败
 */
uint16_t proto_pack(uint8_t addr, uint8_t cmd, const uint8_t *data, uint16_t data_len,
                    uint8_t *frame_buf, uint16_t buf_size);

/**
 * 解析帧为可读字符串（用于日志调试）
 * @param frame     协议帧
 * @param buf       输出缓冲区
 * @param buf_size  缓冲区大小
 * @return 字符串长度
 */
uint16_t proto_frame_to_str(const proto_frame_t *frame, char *buf, uint16_t buf_size);

/**
 * 将字节数组转换为十六进制字符串
 * @param data      数据指针
 * @param len       数据长度
 * @param buf       输出缓冲区
 * @param buf_size  缓冲区大小
 * @return 转换后的字符串长度
 */
uint16_t bytes_to_hex(const uint8_t *data, uint16_t len, char *buf, uint16_t buf_size);

/**
 * 从十六进制字符串解析字节数组
 * @param hex_str   十六进制字符串（空格分隔或连续）
 * @param buf       输出缓冲区
 * @param buf_size  缓冲区大小
 * @return 实际转换的字节数
 */
uint16_t hex_to_bytes(const char *hex_str, uint8_t *buf, uint16_t buf_size);

#endif /* PROTO_PACKER_H */
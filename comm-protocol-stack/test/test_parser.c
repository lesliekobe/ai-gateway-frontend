/**
 * @file test_parser.c
 * @brief 协议解析器测试
 *
 * 测试帧格式：
 * AA BB [LL HH] [ADDR] [CMD] [DATA...] [CRC16] CC DD
 */
#include <stdio.h>
#include <string.h>
#include "../src/proto/proto_parser.h"
#include "../src/proto/proto_packer.h"
#include "../src/core/ringbuffer.h"
#include "../src/core/crc.h"

/* 测试用例：构建合法帧并解析 */
static int test_valid_frame(void) {
    printf("\n=== Test: Valid Frame Parsing ===\n");
    
    /* 构建测试帧 */
    uint8_t addr = 0x02;
    uint8_t cmd = PROTO_CMD_READ;
    uint8_t data[] = { 0x00, 0x10, 0x00, 0x20 };
    
    uint8_t frame[64];
    uint16_t frame_len = proto_pack(addr, cmd, data, sizeof(data), frame, sizeof(frame));
    
    printf("Built frame (%d bytes): ", frame_len);
    for (uint16_t i = 0; i < frame_len; i++)
        printf("%02X ", frame[i]);
    printf("\n");
    
    /* 创建环形缓冲区并写入帧 */
    uint8_t rb_buf[256];
    ringbuffer_t rb;
    ringbuffer_init(&rb, rb_buf, sizeof(rb_buf));
    
    /* 模拟分两次写入（测试粘包） */
    ringbuffer_write(&rb, frame, frame_len / 2);
    ringbuffer_write(&rb, frame + frame_len / 2, frame_len - frame_len / 2);
    
    /* 解析帧 */
    proto_parser_t parser;
    proto_parser_init(&parser);
    proto_frame_t out_frame;
    
    int ret = proto_parser_parse_rb(&parser, &rb, &out_frame);
    
    if (ret == 0) {
        printf("[PASS] Frame parsed successfully\n");
        printf("  addr=0x%02X cmd=0x%02X len=%u\n", 
               out_frame.addr, out_frame.cmd, out_frame.data_len);
        printf("  data: ");
        for (uint16_t i = 0; i < out_frame.data_len; i++)
            printf("%02X ", out_frame.data[i]);
        printf("\n");
        return 0;
    } else {
        printf("[FAIL] Frame parsing failed, ret=%d\n", ret);
        return -1;
    }
}

/* 测试用例：帧头同步 */
static int test_frame_sync(void) {
    printf("\n=== Test: Frame Sync (with junk bytes) ===\n");
    
    /* 构建测试帧 */
    uint8_t data[] = { 0x11, 0x22 };
    uint8_t frame[64];
    uint16_t frame_len = proto_pack(0x03, PROTO_CMD_WRITE, data, sizeof(data), frame, sizeof(frame));
    
    /* 模拟输入：前面有干扰字节 */
    uint8_t junk[] = { 0x00, 0xFF, 0x55, 0xAA, 0xAA, 0xBB };  /* 最后一个AA是帧头起始 */
    uint8_t input[32];
    memcpy(input, junk, sizeof(junk));
    memcpy(input + sizeof(junk), frame, frame_len);
    
    printf("Input (%d bytes): ", (int)(sizeof(junk) + frame_len));
    for (uint16_t i = 0; i < sizeof(junk) + frame_len; i++)
        printf("%02X ", input[i]);
    printf("\n");
    
    /* 创建环形缓冲区 */
    uint8_t rb_buf[256];
    ringbuffer_t rb;
    ringbuffer_init(&rb, rb_buf, sizeof(rb_buf));
    ringbuffer_write(&rb, input, sizeof(junk) + frame_len);
    
    /* 解析帧 */
    proto_parser_t parser;
    proto_parser_init(&parser);
    proto_frame_t out_frame;
    
    int ret = proto_parser_parse_rb(&parser, &rb, &out_frame);
    
    if (ret == 0 && out_frame.addr == 0x03 && out_frame.cmd == PROTO_CMD_WRITE) {
        printf("[PASS] Frame synced correctly despite junk bytes\n");
        return 0;
    } else {
        printf("[FAIL] Frame sync failed, ret=%d\n", ret);
        return -1;
    }
}

/* 测试用例：粘包处理 */
static int test_concat_frames(void) {
    printf("\n=== Test: Concat Frames (Packet Sticking) ===\n");
    
    /* 构建两个帧 */
    uint8_t frame1[64], frame2[64];
    uint16_t len1 = proto_pack(0x01, PROTO_CMD_HEARTBEAT, NULL, 0, frame1, sizeof(frame1));
    uint16_t len2 = proto_pack(0x02, PROTO_CMD_READ, (uint8_t[]){0x00}, 1, frame2, sizeof(frame2));
    
    /* 拼接 */
    uint8_t combined[128];
    memcpy(combined, frame1, len1);
    memcpy(combined + len1, frame2, len2);
    
    printf("Combined frames (%d + %d = %d bytes)\n", len1, len2, len1 + len2);
    
    /* 环形缓冲区 */
    uint8_t rb_buf[256];
    ringbuffer_t rb;
    ringbuffer_init(&rb, rb_buf, sizeof(rb_buf));
    ringbuffer_write(&rb, combined, len1 + len2);
    
    /* 解析第一个帧 */
    proto_parser_t parser;
    proto_parser_init(&parser);
    proto_frame_t out_frame;
    
    int ret = proto_parser_parse_rb(&parser, &rb, &out_frame);
    if (ret == 0) {
        printf("[PASS] First frame: addr=0x%02X cmd=0x%02X\n", 
               out_frame.addr, out_frame.cmd);
    } else {
        printf("[FAIL] First frame parsing failed\n");
        return -1;
    }
    
    /* 解析第二个帧 */
    ret = proto_parser_parse_rb(&parser, &rb, &out_frame);
    if (ret == 0) {
        printf("[PASS] Second frame: addr=0x%02X cmd=0x%02X\n", 
               out_frame.addr, out_frame.cmd);
    } else {
        printf("[FAIL] Second frame parsing failed\n");
        return -1;
    }
    
    return 0;
}

/* 测试用例：CRC错误检测 */
static int test_crc_error(void) {
    printf("\n=== Test: CRC Error Detection ===\n");
    
    /* 构建帧并人为破坏CRC */
    uint8_t data[] = { 0xAA, 0xBB };
    uint8_t frame[64];
    uint16_t frame_len = proto_pack(0x05, PROTO_CMD_REPORT, data, sizeof(data), frame, sizeof(frame));
    
    printf("Original frame CRC: %02X%02X\n", frame[frame_len - 4], frame[frame_len - 3]);
    
    /* 篡改数据 */
    frame[6] ^= 0xFF;  /* 翻转数据位 */
    
    /* 写入缓冲区 */
    uint8_t rb_buf[256];
    ringbuffer_t rb;
    ringbuffer_init(&rb, rb_buf, sizeof(rb_buf));
    ringbuffer_write(&rb, frame, frame_len);
    
    /* 解析 - 应该失败或跳过 */
    proto_parser_t parser;
    proto_parser_init(&parser);
    proto_frame_t out_frame;
    
    int ret = proto_parser_parse_rb(&parser, &rb, &out_frame);
    
    if (ret != 0) {
        printf("[PASS] CRC error correctly detected, ret=%d\n", ret);
        return 0;
    } else {
        printf("[FAIL] CRC error NOT detected (security issue!)\n");
        return -1;
    }
}

int main(void) {
    printf("========================================\n");
    printf("   Protocol Parser Test Suite\n");
    printf("========================================\n");
    
    int passed = 0;
    int total = 4;
    
    if (test_valid_frame() == 0) passed++;
    if (test_frame_sync() == 0) passed++;
    if (test_concat_frames() == 0) passed++;
    if (test_crc_error() == 0) passed++;
    
    printf("\n========================================\n");
    printf("Results: %d/%d tests passed\n", passed, total);
    printf("========================================\n");
    
    return (passed == total) ? 0 : 1;
}
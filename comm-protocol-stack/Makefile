# Makefile for Comm Protocol Stack

CC = gcc
CFLAGS = -Wall -Wextra -I./src -std=c11 -Os
LDFLAGS =

# 源文件
CORE_SRC = src/core/ringbuffer.c src/core/crc.c
PROTO_SRC = src/proto/proto_parser.c src/proto/proto_packer.c src/proto/session.c src/proto/proto_stack.c
TRANSPORT_SRC = src/transport/transport_uart.c src/transport/transport_socket.c

ALL_SRC = $(CORE_SRC) $(PROTO_SRC) $(TRANSPORT_SRC)

# 示例程序
EXAMPLES = example_uart example_tcp

.PHONY: all clean test

all: $(EXAMPLES)

example_uart: example/example_uart.c $(ALL_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(ALL_SRC) $(LDFLAGS)

example_tcp: example/example_tcp.c $(ALL_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(ALL_SRC) $(LDFLAGS)

# 单元测试
test: test_ringbuffer test_crc test_parser

test_ringbuffer: test/test_ringbuffer.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(CORE_SRC) $(LDFLAGS)
	./$@

test_crc: test/test_crc.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(CORE_SRC) $(LDFLAGS)
	./$@

test_parser: test/test_parser.c $(ALL_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(ALL_SRC) $(LDFLAGS)
	./$@

clean:
	rm -f $(EXAMPLES) test_ringbuffer test_crc test_parser
	find . -name "*.o" -delete
/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zephyr.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <board.h>
#include <uart.h>
#include <misc/byteorder.h>

#define BUF_MAXSIZE	256
#define SLEEP_TIME	500

static struct device *uart1_dev;
static uint8_t rx_buf[BUF_MAXSIZE];
static uint8_t tx_buf[BUF_MAXSIZE];
static uint8_t nci_reset[] = {0x20, 0x00, 0x01, 0x00};

static void msg_dump(const char *s, uint8_t *data, unsigned len)
{
	unsigned i;

	printf("%s: ", s);
	for (i = 0; i < len; i++) {
		printf("%02x ", data[i]);
	}
	printf("(%u bytes)\n", len);
}

static void uart1_isr(struct device *x)
{
	int len = uart_fifo_read(uart1_dev, rx_buf, BUF_MAXSIZE);

	ARG_UNUSED(x);
	msg_dump(__func__, rx_buf, len);
}

static void uart1_init(void)
{
	uart1_dev = device_get_binding("UART_1");

	uart_irq_callback_set(uart1_dev, uart1_isr);
	uart_irq_rx_enable(uart1_dev);

	printf("%s() done\n", __func__);
}

void main(void)
{
	uint32_t *size = (uint32_t *)tx_buf;

	printf("Sample app running on: %s\n", CONFIG_ARCH);

	uart1_init();

	/* 4 bytes for the payload's length */
	UNALIGNED_PUT(sys_cpu_to_be32(sizeof(nci_reset)), size);

	/* NFC Controller Interface reset cmd */
	memcpy(tx_buf + sizeof(uint32_t), nci_reset, sizeof(nci_reset));

	/*
	 * Peer will receive: 0x00 0x00 0x00 0x04 0x20 0x00 0x01 0x00
	 *	                nci_reset size   +    nci_reset cmd
	 */
	uart_fifo_fill(uart1_dev, tx_buf, sizeof(uint32_t) + sizeof(nci_reset));

	while (1) {
		k_sleep(SLEEP_TIME);
	}
}

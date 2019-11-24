/*
 * MIT License
 *
 * Copyright (c) 2019 Francesco Lavra <francescolavra.fl@gmail.com>
 * and Contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _STM32LWIPOPTS_H
#define _STM32LWIPOPTS_H

/* LwIP configuration options for STM32 */

#define NO_SYS	1

#define SYS_LIGHTWEIGHT_PROT	0

#define LWIP_NOASSERT

#define MEM_ALIGNMENT	4

#define MEM_SIZE	(16 * 1024)

#define MEMP_NUM_PBUF	10

#define MEMP_NUM_UDP_PCB	6

#define MEMP_NUM_TCP_PCB	10

#define MEMP_NUM_TCP_PCB_LISTEN	6

#define MEMP_NUM_TCP_SEG	16

#define MEMP_NUM_SYS_TIMEOUT	10

#define PBUF_POOL_SIZE	8

#define PBUF_POOL_BUFSIZE	1524

#define LWIP_TCP						1
#define TCP_TTL							255
#define LWIP_SO_RCVTIMEO				1
#define LWIP_SO_RCVRCVTIMEO_NONSTANDARD	1
#define LWIP_SO_SNDTIMEO				1
#define LWIP_SO_SNDRCVTIMEO_NONSTANDARD	1

#define TCP_QUEUE_OOSEQ	0

#define TCP_MSS	(1500 - 40)

#define TCP_SND_BUF	(8 * TCP_MSS)

#define TCP_SND_QUEUELEN	(2 * TCP_SND_BUF / TCP_MSS)

#define TCP_WND	(3 * TCP_MSS)

#define LWIP_TCP_KEEPALIVE					1
#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS	1

#define LWIP_ICMP					1
#define LWIP_RAW					1
#define DEFAULT_RAW_RECVMBOX_SIZE	3

#define LWIP_DHCP	1

#define LWIP_UDP	1
#define UDP_TTL		255

#define LWIP_STATS			0
#define LWIP_PROVIDE_ERRNO

#define LWIP_NETIF_HOSTNAME			1
#define LWIP_NETIF_STATUS_CALLBACK	1
#define LWIP_NETIF_LINK_CALLBACK	1
#define LWIP_DHCP_CHECK_LINK_UP		1

#define CHECKSUM_BY_HARDWARE

#define CHECKSUM_GEN_IP		0
#define CHECKSUM_GEN_UDP	0
#define CHECKSUM_GEN_TCP	0
#define CHECKSUM_CHECK_IP	0
#define CHECKSUM_CHECK_UDP	0
#define CHECKSUM_CHECK_TCP	0
#define CHECKSUM_GEN_ICMP	0

#define LWIP_NETCONN	0

#define LWIP_SOCKET	0
#define LWIP_DNS	1

#define LWIP_HTTPD_CGI	1

#define LWIP_HTTPD_SSI	1

#define HTTPD_USE_CUSTOM_FSDATA	1

#define ETHERNET_RMII_MODE_CONFIGURATION	1

#endif /* _STM32LWIPOPTS_H */

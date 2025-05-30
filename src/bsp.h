//============================================================================
// Product: DPP example (console)
// Last Updated for Version: 7.3.0
// Date of the Last Update:  2023-08-12
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <www.state-machine.com>
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "sinais.h"      // DPP Application interface
#ifndef BSP_H_
#define BSP_H_

#define BSP_TICKS_PER_SEC 100U
#define BUFLEN 512  //Max length of buffer
#define PORTIN 777  //The port on which to listen for incoming data
#define PORTOUT 7777

#define LCP_CODE_CONFIGURE_REQUEST    0x01  /**< Configure-Request     */  // :contentReference[oaicite:0]{index=0}
#define LCP_CODE_CONFIGURE_ACK        0x02  /**< Configure-Ack         */  // :contentReference[oaicite:1]{index=1}
#define LCP_CODE_PAYLOAD         0x0F  // pick an unused code
#define LCP_CODE_PAYLOAD_ACK     0x10

void BSP_init(int argc, char *argv[]);
void BSP_start(void);
void BSP_displayPaused(uint8_t paused);
void BSP_displayPhilStat(uint8_t n, uint8_t st, char const *stat);
void BSP_terminate(int16_t result);
void sendMessage(const char *fmt, ...);
void messageAck(uint8_t identifier);

void BSP_randomSeed(uint32_t seed); // random seed
uint32_t BSP_random(void);          // pseudo-random generator
void send_up(void);
void send_configure_request(void);
void send_configure_ack(uint8_t identifier);
extern void send_terminate_ack(const LcpPacket *received);
void send_terminate_request(void);
void assert_failed(char const *const module, int_t const id); // prototype


void bsp_on();
void bsp_off();
void BSP_forno(int i);
void BSP_luz(int i);
void BSP_digito(int seg, int num);

void BSP_sendPPPRaw(uint8_t code, const uint8_t *payload, uint16_t payload_len);
#endif // BSP_H_

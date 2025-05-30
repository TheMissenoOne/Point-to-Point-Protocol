//============================================================================
// Product: BSP for DPP example (console)
// Last updated for version 8.0.0
// Last updated on  2024-09-18
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
#include "qpc.h"      // QP/C real-time embedded framework
#include "sinais.h"      // DPP Application interface
#include "bsp.h"      // Board Support Package

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <windows.h>
#include <pthread.h>
#include "safe_std.h" // portable "safe" <stdio.h>/<string.h> facilities
#define MAX_PAYLOAD_BLOCKS  20

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static uint32_t l_rnd; // random seed
const char *ppp_signals[] = { "UP", "OPEN", "DOWN", "CLOSE", "RCR+", "RCR-",
		"RCA", "RCN", "RTR", "TO+", "TO-" };

static uint32_t l_rnd;
struct sockaddr_in si_other;
static SOCKET s;

WSADATA wsaData;
LPHOSTENT lpHostEntry;
pthread_t tudpServer;

extern int build_lcp_packet(LcpPacket *pkt, uint8_t code, uint8_t identifier);

void LCPEvt_init(LCPEvt *const me, QSignal sig, uint8_t identifier) {
	// inicializa o evento-base
	QEvt_init(&me->super, sig);
	// guarda o identificador de pacote
	me->identifier = identifier;
}

// UDP server thread

int BSP_recvPPPRaw(uint8_t *buffer, uint16_t max_len) {
	char tmp[600];
	struct sockaddr_in from;
	int fromlen = sizeof(from);

	int len = recvfrom(s, tmp, sizeof(tmp), 0, (struct sockaddr*) &from,
			&fromlen);
	if (len == SOCKET_ERROR) {
		return -WSAGetLastError();  // negative on error
	}
	// copy up to max_len
	int copy = (len < max_len) ? len : max_len;
	memcpy(buffer, tmp, copy);

	return copy;
}

static void* udpServer(void *arg) {
	uint8_t buf[BUFLEN];
	int recv_len;

	(void) arg;  // unused

	while (1) {
		// 1) block waiting for the next PPP frame
		recv_len = BSP_recvPPPRaw(buf, sizeof(buf));
		if (recv_len <= 0) {
			// error or no data
			continue;
		}

		// 2) dump the raw frame
		printf("[BSP] Frame recebido (%d bytes)\n", recv_len);
		for (int i = 0; i < recv_len; ++i) {
			printf("%02X ", buf[i]);
		}
		printf("\n");

		// 3) sanity‐check HDLC flags/address/control/protocol
		if (recv_len < 6 || buf[0] != 0x7E || buf[recv_len - 1] != 0x7E) {
			printf("Frame inválido (flags incorretas ou muito curto)\n");
			continue;
		}
		size_t pos = 1;
		if (buf[pos++] != 0xFF || buf[pos++] != 0x03) {
			printf("Endereço/controle inválido\n");
			continue;
		}
		uint8_t proto_hi = buf[pos++];
		uint8_t proto_lo = buf[pos++];
		uint16_t proto = ((uint16_t) proto_hi << 8) | proto_lo;
		if (proto != 0xC021) {
			printf("Protocolo não suportado: 0x%04X\n", proto);
			continue;
		}

		size_t payload_len = (size_t) recv_len - pos - 3;
		if (payload_len < 4) {
			printf("Payload LCP muito curto\n");
			continue;
		}

		//  extraímos o cabeçalho LCP do payload

		// code, id, length
		uint8_t code = buf[pos++];
		uint8_t identifier = buf[pos++];
		uint8_t len_hi = buf[pos++];
		uint8_t len_lo = buf[pos++];
		uint16_t length = ((uint16_t) len_hi << 8) | len_lo;
		size_t data_len = length < 4 ? 0 : length - 4;
		const uint8_t *data = &buf[pos];

		fflush(stdout);

		// processa conforme o código LCP
		switch (code) {
		case LCP_CODE_CONFIGURE_REQUEST: {
			LCPEvt *e = (LCPEvt*) QF_newX_(sizeof(LCPEvt),
			QF_NO_MARGIN, RCR_PLUS_SIG);
			if (e != NULL) {
				LCPEvt_init(e, RCR_PLUS_SIG, identifier);
				QACTIVE_PUBLISH((QEvt* )e, NULL);
			}
			break;
		}
		case LCP_CODE_CONFIGURE_ACK: {
			LCPEvt *e = (LCPEvt*) QF_newX_(sizeof(LCPEvt),
			QF_NO_MARGIN, RCA_SIG);
			if (e != NULL) {
				LCPEvt_init(e, RCA_SIG, identifier);
				QACTIVE_PUBLISH((QEvt* )e, NULL);
			}
			break;
		}
		case LCP_CODE_PAYLOAD: {
			// calculate exact size of the PayloadEvt including the actual payload
			size_t evtSize = sizeof(PayloadEvt)
					- sizeof(((PayloadEvt*) 0)->data)  // overhead
			+ data_len;
			printf("[BSP] Mensagem recebida (id=%u, len=%zu): ", identifier,
					data_len);
//			printf("%.*s\n", (int)data_len, (char*)data);
//			printf("[BSP] PayloadEvt: total=%zu, data_len=%u\n", evtSize, data_len);
			fflush(stdout);
			PayloadEvt *e = (PayloadEvt*) QF_newX_(evtSize, QF_NO_MARGIN,
					PL_SIG);
			if (e != NULL) {
				QEvt_init(&e->super, PL_SIG);
				e->identifier = identifier;
				e->length = data_len;
				memcpy(e->data, data, data_len);
				QACTIVE_POST(AO_PPP, (QEvt* )e, BSP_PRIO); // <<< direct post to PPP
			}
			break;
		}
		case LCP_CODE_PAYLOAD_ACK: {
			LCPEvt *e = (LCPEvt*) QF_newX_(sizeof(LCPEvt),
			QF_NO_MARGIN, PLA_SIG);
			if (e != NULL) {
				LCPEvt_init(e, PLA_SIG, identifier);
				QACTIVE_PUBLISH((QEvt* )e, NULL);
			}
			break;
		}
		default: {
			printf("Código LCP desconhecido: 0x%02X\n", code);
			break;
		}
		}
	}

	return NULL;
}

void BSP_startUDPServer(void) {
	printf("[BSP] Inicializando UDP Server \n");
	fflush(stdout);
	pthread_create(&tudpServer, NULL, udpServer, NULL);
}

//============================================================================
Q_NORETURN Q_onError(char const *const module, int_t const id) {
	QS_ASSERTION(module, id, 10000U); // report assertion to QS
	QF_onCleanup();
	QS_EXIT();
	exit(-1);
}
//............................................................................

void assert_failed(char const *const module, int_t const id) {
	Q_onError(module, id);
}

//============================================================================
void BSP_init(int argc, char *argv[]) {
	Q_UNUSED_PAR(argc);
	Q_UNUSED_PAR(argv);

	WSAStartup(MAKEWORD(2, 1), &wsaData);

// create the one-and-only UDP socket:
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		printf("socket() falhou: %d\n", WSAGetLastError());
		Q_ERROR();
	}

// bind it for receive on PORTIN:
	struct sockaddr_in me = { 0 };
	me.sin_family = AF_INET;
	me.sin_port = htons(PORTIN);
	me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, (struct sockaddr*) &me, sizeof(me)) == SOCKET_ERROR) {
		printf("bind() falhou: %d\n", WSAGetLastError());
		Q_ERROR();
	}

// set up si_other (the peer address/port) for sendto():
	lpHostEntry = gethostbyname("127.0.0.1");
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORTOUT);
	si_other.sin_addr = *((LPIN_ADDR) * lpHostEntry->h_addr_list);

	BSP_startUDPServer();

}
//............................................................................
void BSP_start(void) {
	// 1) PPPEvt pool
	static QF_MPOOL_EL(PPPEvt)
	smlPoolSto[POOLSIZE];
	QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

	// 2) PayloadEvt pool
	static QF_MPOOL_EL(PayloadEvt)
	payloadPoolSto[MAX_PAYLOAD_BLOCKS];
	QF_poolInit(payloadPoolSto, sizeof(payloadPoolSto),
			sizeof(payloadPoolSto[0]));

	// 3) (oops) publish‐subscribe init **twice**
	static QSubscrList subscrSto[MAX_PUB_SIG];
	QActive_psInit(subscrSto, Q_DIM(subscrSto));

	// 4) start the AO
	static QEvtPtr l_microQueueSto[QUEUESIZE];
	PPP_ctor();

	QActive_start(AO_PPP, 7U,            // QP prio. of the AO
			l_microQueueSto,           // event queue storage
			Q_DIM(l_microQueueSto),    // queue length [events]
			(void*) 0, 0U,           // no stack storage
			(void*) 0);
}
//............................................................................
void BSP_terminate(int16_t result) {
	(void) result;
	QF_stop(); // stop the main "ticker thread"
}

//............................................................................
uint32_t BSP_random(void) { // a very cheap pseudo-random-number generator
// "Super-Duper" Linear Congruential Generator (LCG)
// LCG(2^32, 3*7*11*13*23, 0, seed)
//
	uint32_t rnd = l_rnd * (3U * 7U * 11U * 13U * 23U);
	l_rnd = rnd;
	return rnd >> 8;
}
//............................................................................
void BSP_randomSeed(uint32_t seed) {
	l_rnd = seed;
}

//============================================================================
#if CUST_TICK
#include <sys/select.h> // for select() call used in custom tick processing
#endif

void QF_onStartup(void) {
	QF_consoleSetup();

#if CUST_TICK
    // disable the standard clock-tick service by setting tick-rate to 0
    QF_setTickRate(0U, 10U); // zero tick-rate / ticker thread prio.
#else
	QF_setTickRate(BSP_TICKS_PER_SEC, 50); // desired tick rate/ticker-prio
#endif
}
//............................................................................
void QF_onCleanup(void) {
	printf("\n%s\n", "Bye! Bye!");
	QF_consoleCleanup();
}
//............................................................................

void QF_onClockTick(void) {

#if CUST_TICK
    // NOTE:
    // The standard clock-tick service has been DISABLED in QF_onStartup()
    // by setting the clock tick rate to zero.
    // Therefore QF_onClockTick() must implement an alternative waiting
    // mechanism for the clock period. This particular implementation is
    // based on the select() system call to block for the desired timeout.

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = (1000000/BSP_TICKS_PER_SEC);
    select(0, NULL, NULL, NULL, &tv); // block for the timevalue
#endif
	QTIMEEVT_TICK_X(0U, &l_clock_tick); // process time events at rate 0
	QS_RX_INPUT(); // handle the QS-RX input
	QS_OUTPUT();   // handle the QS output
	switch (QF_consoleGetKey()) {
	case '\33': { // ESC pressed?
		BSP_terminate(0);
		break;
	}
	default: {
		break;
	}
	}
}
//============================================================================
#ifdef Q_SPY // define QS callbacks

//............................................................................
//! callback function to execute user commands
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}
#endif // Q_SPY

int build_lcp_packet(LcpPacket *packet, uint8_t code, uint8_t identifier) {

	packet->code = code;
	packet->identifier = identifier;
	packet->length = htons(4); // 4 bytes de cabeçalho + opções

	return 4; // tamanho total do pacote
}

/// Função muito simples de CRC-16-IBM (pode trocar por outra)
static uint16_t crc16(const uint8_t *buf, size_t len) {
	uint16_t crc = 0xFFFF;
	while (len--) {
		crc ^= *buf++ << 8;
		for (int i = 0; i < 8; ++i) {
			if (crc & 0x8000)
				crc = (crc << 1) ^ 0x1021;
			else
				crc <<= 1;
		}
	}
	return crc;
}

void BSP_sendPPPRaw(uint8_t code, const uint8_t *payload, uint16_t payload_len) {
	uint8_t frame[600];
	size_t pos = 0;

	// 1) Flag de início
	frame[pos++] = 0x7E;

	// 2) Address
	frame[pos++] = 0xFF;

	// 3) Control
	frame[pos++] = 0x03;

	// 4) Protocol (big-endian). Aqui usamos LCP = 0xC021
	frame[pos++] = 0xC0;
	frame[pos++] = 0x21;

	// 5) Code (1 byte) + Payload
	frame[pos++] = code;
	if (payload_len > 0 && payload != NULL) {
		memcpy(&frame[pos], payload, payload_len);
		pos += payload_len;
	}

	// 6) FCS (CRC-16). Aplica sobre Address..(Code+Payload)
	//    total bytes under FCS = 1(addr) + 1(ctrl) + 2(proto) + 1(code) + payload_len
	int fcs = crc16(&frame[1], 1 + 1 + 2 + 1 + payload_len);
	frame[pos++] = (uint8_t) ((fcs >> 8) & 0xFF);
	frame[pos++] = (uint8_t) (fcs & 0xFF);

	// 7) Flag de fim
	frame[pos++] = 0x7E;

	// envia via UDP
	int sent = sendto(s, (const char*) frame,  // data buffer
			(int) pos,             // length
			0,                    // flags
			(struct sockaddr*) &si_other, (int) sizeof(si_other));
	if (sent == SOCKET_ERROR) {
		int err = WSAGetLastError();
		printf("[BSP] Erro no sendto: %d\n", err);
	}

	// Debug print of the frame
	printf("[BSP] PPP frame enviado (%zu bytes):\n    ", pos);
	for (size_t i = 0; i < pos; ++i) {
		printf("%02X ", frame[i]);
	}
	printf("\n");
}

void sendMessage(const char *fmt, ...) {
	// 1) format the text payload
	char text[256];
	va_list va;
	va_start(va, fmt);
	int len = vsnprintf(text, sizeof(text), fmt, va);
	va_end(va);
	if (len <= 0) {
		return;              // nothing to send
	}
	if (len > 250)
		len = 250; // leave room for header

	// 2) bump the identifier (wrap at 255→1)
	static uint8_t lcp_payload_id = 0;
	lcp_payload_id = (lcp_payload_id == 255) ? 1 : (lcp_payload_id + 1);

	// 3) build an LCP header + data buffer
	uint8_t buffer[4 + sizeof(text)];
	uint16_t total_len = (uint16_t) (4 + len);
	buffer[0] = LCP_CODE_PAYLOAD;         // 0x0F
	buffer[1] = lcp_payload_id;           // new identifier
	buffer[2] = (uint8_t) (total_len >> 8);
	buffer[3] = (uint8_t) (total_len & 0xFF);
	memcpy(&buffer[4], text, len);

	// 4) send the framed packet
	BSP_sendPPPRaw(LCP_CODE_PAYLOAD, buffer + 1, (uint16_t) (3 + len));
	//               ^^^^^^^^^^^^^^^
	// we pass “identifier+length+data” as the payload,
	// since BSP_sendPPPRaw already emits the code byte separately

	printf("[BSP] Enviado payload (id=%u, len=%u): %.*s\n", lcp_payload_id, len,
			len, text);
}

// Send a “payload‐ack” for a given identifier

// Send an arbitrary fully‐built LCP packet via the old send_lcp_packet
// (you can drop this if you now always use BSP_sendPPPRaw directly)
void send_lcp_packet(const char *ip, uint16_t port, LcpPacket *packet,
		int packet_len) {
	(void) ip;
	(void) port; // we assume BSP_init already set si_other
	BSP_sendPPPRaw(packet->code, (const uint8_t*) &packet->identifier,
			ntohs(packet->length) - 1);
	// note: we subtract 1 because code byte is passed separately
}

void messageAck(uint8_t identifier) {
	LcpPacket ack;
	ack.code = LCP_CODE_PAYLOAD_ACK;
	ack.identifier = identifier;
	ack.length = htons(4);
	send_lcp_packet("127.0.0.1", PORTOUT, &ack, sizeof(ack));
	printf("[BSP] LCP Payload‐Ack enviado (id=%u)\n", identifier);
}

// Send a Configure‐Request (you’ll want to pass real options here)
void send_configure_request(void) {
	uint8_t header[4];
	uint16_t total_len = htons(4);
	header[0] = LCP_CODE_CONFIGURE_REQUEST;
	header[1] = 42;          // or your chosen identifier
	memcpy(&header[2], &total_len, 2);

	BSP_sendPPPRaw(LCP_CODE_CONFIGURE_REQUEST, &header[1], 3);

	printf("Configure-Request Enviado\n");
}

void send_configure_ack(uint8_t identifier) {
	LcpPacket ack;
	ack.code = LCP_CODE_CONFIGURE_ACK;
	ack.identifier = identifier;
	ack.length = htons(4);
	send_lcp_packet("127.0.0.1", PORTOUT, &ack, sizeof(ack));
	printf("Configure Ack Enviado [id=%u]\n", identifier);
}


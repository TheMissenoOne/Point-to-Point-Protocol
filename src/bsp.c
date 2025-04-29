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
//#include <pthread.h>
#include "safe_std.h" // portable "safe" <stdio.h>/<string.h> facilities

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static uint32_t l_rnd; // random seed
const char *ppp_signals[] = {
	"UP",
	"OPEN",
	"DOWN",
	"CLOSE",
    "RCR+",
    "RCR-",
    "RCA",
    "RCN",
    "RTR",
    "TO+",
    "TO-"
};

static uint32_t l_rnd;
struct sockaddr_in si_other;
int s;
WSADATA wsaData;
LPHOSTENT lpHostEntry;
//pthread_t tudpServer;

// Send PPP signal over UDP
static void sendPPPSig(int idx) {

	int slen = (int) sizeof(si_other);
	if (s != -1){
		sendto(s, ppp_signals[idx], (int) strlen(ppp_signals[idx]), 0, (struct sockaddr*) &si_other, slen);
		printf("PPP signal sent: %s\n", ppp_signals[idx]);
		fflush(stdout);
	}
}

void send_lcp_packet(const char* ip, uint16_t port, LcpPacket* packet, int packet_len) {

	int slen = (int) sizeof(si_other);
	if (s != -1){
		sendto(s, packet, packet_len, 0, (struct sockaddr*) &si_other, slen);

		printf("Packet LCD\n");
		printf("--------------------\n");
		print_packet_hex(packet, packet_len);
	}
}

// UDP server thread
DWORD WINAPI udpServer(void *arg) {

	struct sockaddr_in si_other;
	struct sockaddr_in si_me;
	int slen = (int) sizeof(si_other);
	int recv_len;
	char buf[BUFLEN];

	int ss = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ss != -1) {

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORTIN);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(ss, (struct sockaddr *) &si_me, sizeof(si_me)) != -1) {

	while (1) {
		memset(buf, 0, BUFLEN);
		if ((recv_len = recvfrom(ss, buf, BUFLEN, 0, (struct sockaddr*) &si_other, &slen)) != -1) {

        	printf("Recebido: %s\n", buf);
        	fflush(stdout);
			buf[recv_len] = '\0';

			int sinal = 0;
			for (int i = 0; i < 11; i++)
			{
			    if (strncmp(buf, ppp_signals[i], strlen(ppp_signals[i])) == 0) {

			    sinal = 1;

				// Posta evento conforme o código acima
				switch (i){
					case 0: {
					    PPPEvt *evt = Q_NEW(PPPEvt, UP_SIG, NULL);
					    QACTIVE_PUBLISH((QEvt *)evt, NULL);
					    break;
					}
					case 1: {
					    PPPEvt *evt = Q_NEW(PPPEvt, DOWN_SIG, NULL);
					    QACTIVE_PUBLISH((QEvt *)evt, NULL);
					    break;
					}
					case 2: {
					    PPPEvt *evt = Q_NEW(PPPEvt, OPEN_SIG, NULL);
					    QACTIVE_PUBLISH((QEvt *)evt, NULL);
					    break;
					}
					case 3: {
					    PPPEvt *evt = Q_NEW(PPPEvt, CLOSE_SIG, NULL);
					    QACTIVE_PUBLISH((QEvt *)evt, NULL);
					    break;
					}
					case 4: {
					    PPPEvt *evt = Q_NEW(PPPEvt, RCR_PLUS_SIG, NULL);
					    QACTIVE_PUBLISH((QEvt *)evt, NULL);
					    break;
					}
					case 5: {
					    PPPEvt *evt = Q_NEW(PPPEvt, RCR_MINUS_SIG, NULL);
					    QACTIVE_PUBLISH((QEvt *)evt, NULL);
					    break;
					}
					case 6: {
					    PPPEvt *evt = Q_NEW(PPPEvt, RCA_SIG, NULL);
					    QACTIVE_PUBLISH((QEvt *)evt, NULL);
					    break;
					}
					case 7: {
						PPPEvt *evt = Q_NEW(PPPEvt, RCN_SIG, NULL);
						QACTIVE_PUBLISH((QEvt *)evt, NULL);
						break;
					}
					case 8: {
						PPPEvt *evt = Q_NEW(PPPEvt, RTR_SIG, NULL);
						QACTIVE_PUBLISH((QEvt *)evt, NULL);
						break;
					}
					case 9: {
						PPPEvt *evt = Q_NEW(PPPEvt, RTR_SIG, NULL);
						QACTIVE_PUBLISH((QEvt *)evt, NULL);
						break;
					}
					case 10: {
						PPPEvt *evt = Q_NEW(PPPEvt, TO_PLUS_SIG, NULL);
						QACTIVE_PUBLISH((QEvt *)evt, NULL);
						break;
					}
					case 11: {
						PPPEvt *evt = Q_NEW(PPPEvt, TO_MINUS_SIG, NULL);
						QACTIVE_PUBLISH((QEvt *)evt, NULL);
						break;
					}
				}
    		}
    	}

		LcpPacket *pkt = (LcpPacket *) buf;

		if(!sinal)
		{
			switch (pkt->code) {
			    case LCP_CODE_CONFIGURE_REQUEST: {
			    	LCPEvt *lcp_request = Q_NEW(LCPEvt, RCR_PLUS_SIG, pkt->identifier);
				    QACTIVE_PUBLISH((QEvt *)lcp_request, NULL);
			        break;
			    }
			    case LCP_CODE_CONFIGURE_ACK:{
			    	LCPEvt *lcp_request_ack = Q_NEW(LCPEvt, RCA_SIG, pkt->identifier);
			    	QACTIVE_PUBLISH((QEvt *)lcp_request_ack, NULL);
			        break;
			    }
			    case LCP_CODE_TERMINATE_REQUEST:{
			    	LCPEvt *lcp_request_terminate = Q_NEW(LCPEvt, RCN_SIG, pkt->identifier);
			    	QACTIVE_PUBLISH((QEvt *)lcp_request_terminate, NULL);
			        break;
			    }
			    default:
			        printf("Código desconhecido: 0x%02X\n", pkt->identifier);
			        break;
			}

			sinal = 0;
		}
    }
}}}
return NULL;
}

//============================================================================
Q_NORETURN Q_onError(char const * const module, int_t const id) {
    QS_ASSERTION(module, id, 10000U); // report assertion to QS
    QF_onCleanup();
    QS_EXIT();
    exit(-1);
}
//............................................................................

void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

//============================================================================
void BSP_init(int argc, char *argv[]) {
    Q_UNUSED_PAR(argc);
    Q_UNUSED_PAR(argv);

    BSP_randomSeed(1234U);

    // initialize the QS software tracing
    if (QS_INIT((argc > 1) ? argv[1] : (void *)0) == 0U) {
        Q_ERROR();
    }

    QS_OBJ_DICTIONARY(&l_clock_tick);

    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);

    QS_ONLY(produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QS_ALL_RECORDS);
    QS_GLB_FILTER(-QS_QF_TICK);    // exclude the tick record

	WSAStartup(MAKEWORD(2,1),&wsaData);
    lpHostEntry = gethostbyname("127.0.0.1");
	s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORTOUT);
	si_other.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	//pthread_create(&tudpServer,NULL,udpServer,NULL);

	HANDLE tudpServer = CreateThread(
	        NULL,       // atributos padrão
	        0,          // tamanho do stack (0 = padrão)
	        udpServer,  // função da thread
	        NULL,       // parâmetro da função
	        0,          // flags
	        NULL        // ID da thread (opcional)
	    );
}
//............................................................................
void BSP_start(void) {
    // initialize event pools
    static QF_MPOOL_EL(PPPEvt) smlPoolSto[POOLSIZE];
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QSubscrList subscrSto[MAX_PUB_SIG];
    QActive_psInit(subscrSto, Q_DIM(subscrSto));

    // instantiate and start AOs/threads...
    static QEvtPtr l_microQueueSto[QUEUESIZE];
    PPP_ctor();

    QActive_start(AO_PPP,
        7U,            // QP prio. of the AO
		l_microQueueSto,           // event queue storage
        Q_DIM(l_microQueueSto),    // queue length [events]
        (void *)0, 0U,           // no stack storage
        (void *)0);

	sendPPPSig(0); // Send UP// no initialization param
}
//............................................................................
void BSP_terminate(int16_t result) {
    (void)result;
    QF_stop(); // stop the main "ticker thread"
}

//............................................................................
uint32_t BSP_random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
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

int build_lcp_packet(LcpPacket* packet, uint8_t code, uint8_t identifier) {

    packet->code = code;
    packet->identifier = identifier;
    packet->length = htons(4); // 4 bytes de cabeçalho + opções

    return 4; // tamanho total do pacote
}

void send_up()
{
	sendPPPSig(0);
}

void send_configure_request() {
	LcpPacket pkt;

	int pkt_len = build_lcp_packet(&pkt, 0x01, 42); // 0x01 = Configure-Request

	send_lcp_packet("127.0.0.1", PORTOUT, &pkt, pkt_len);

    printf("Configure Request (LCP) enviado\n");
}

void send_configure_ack(LcpPacket *received) {

	LcpPacket ack_packet;

    ack_packet.code = LCP_CODE_CONFIGURE_ACK;
    ack_packet.identifier = received->identifier;
    ack_packet.length = received->length;

	send_lcp_packet("127.0.0.1", PORTOUT, &ack_packet, ack_packet.length );

    printf("Configure Request ACK (LCP) enviado\n");
}

// FUnção para imprmir qualquer struct
void print_packet_hex(const void* packet, size_t length) {
    const unsigned char* bytes = (const unsigned char*)packet;
    printf("Packet (%zu bytes):\n", length);
    for (size_t i = 0; i < length; ++i) {
        printf("%02X ", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (length % 16 != 0) printf("\n");
	fflush(stdout);
}


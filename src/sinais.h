/*
 * sinais.h
 *
 *  Created on: 05/08/2013
 *      Author: Amaury
 */

#ifndef SINAIS_H_
#define SINAIS_H_

 #include "qp_port.h"

/* Definição dos sinais (eventos) */
enum PPP_Signals {
	// Controle de link(inicio/fim)
    UP_SIG = Q_USER_SIG,
    DOWN_SIG,
    OPEN_SIG,
    CLOSE_SIG,

	// Sinais do protocolo LCP (Link Control Protocol)
    RCR_PLUS_SIG, // Receive Configure-Request (aceito)
    RCR_MINUS_SIG, // Receive Configure-Request (não aceitável)
    RCA_SIG, // Receive Configure-Ack
    RCN_SIG, // Receive Configure-Nak ou Reject

	// Sinais de terminação
    RTR_SIG,
    RTA_SIG,

	// Sinais de temporização
    TO_PLUS_SIG,
    TO_MINUS_SIG,

    TIME_TICK_SIG, /* terminate the application */
	MAX_SIG
};

#define MAX_PUB_SIG TIME_TICK_SIG+10000
#define QUEUESIZE ((uint8_t)5000)
#define POOLSIZE ((uint8_t)5000)

#pragma pack(push, 1)
typedef struct {
    uint8_t code;         // Código LCP: 0x01=Request, 0x02=Ack, 0x03=Nak, 0x04=Reject
    uint8_t identifier;   // Número da requisição (usado para parear request e ack)
    uint16_t length;      // Tamanho total do pacote (incluindo cabeçalho e options)
} LcpPacket;
#pragma pack(pop)

typedef struct {
    QEvt super;

} PPPEvt;

// public:
static inline PPPEvt * PPPEvt_init(PPPEvt * const me, uint8_t andar)
{
    if (me != (PPPEvt *)0) {
    }

    return me;
}

typedef struct LCPEvtTag {
    QEvt super;

    int packet;
} LCPEvt;

static inline LCPEvt * LCPEvt_init(LCPEvt * const me, int packet)
{
    if (me != (LCPEvt *)0) {
    	me->packet = packet;
    }

    return me;
}

extern QActive * const AO_PPP;
extern void PPP_ctor();

#endif /* SINAIS_H_ */

#ifndef SINAIS_H_
#define SINAIS_H_

#include "qp_port.h"

//--------------------------------------------------------------------------------
// Sinais Genéricos (começando em Q_USER_SIG)
//--------------------------------------------------------------------------------
enum PPP_Signals {
    // Controle de Link (up/down)
    UP_SIG = Q_USER_SIG,
    DOWN_SIG,

    // Evento de abertura/fechamento solicitado pela aplicação
    OPEN_SIG,
    CLOSE_SIG,

    // Sinais LCP (Link Control Protocol)
    PPP_CONFIG_REQ_SIG,  // Configure-Request enviado
    PPP_CONFIG_ACK_SIG,  // Configure-Ack recebido
    PPP_CONFIG_NAK_SIG,  // Configure-Nak/Rej recebido
    PPP_TERM_REQ_SIG,    // Terminate-Request enviado
    PPP_TERM_ACK_SIG,    // Terminate-Ack recebido

    // Sinais LCP internos (recebimento de pacotes)
    RCR_PLUS_SIG,   // Receive-Configure-Request (aceitável)
    RCR_MINUS_SIG,  // Receive-Configure-Request (não aceitável)
    RCA_SIG,        // Receive-Configure-Ack
    RCN_SIG,        // Receive-Configure-Nak ou Reject
	PL_SIG,
	PLA_SIG,

    // Sinais de encerramento de camada
    RTR_SIG,        // Receive-Terminate-Request
    RTA_SIG,        // Receive-Terminate-Ack

    // Temporizadores de retransmissão
    TO_PLUS_SIG,    // Timeout para REQUEST
    TO_MINUS_SIG,   // Timeout para TERMINATE

    // Tick do sistema
    TIME_TICK_SIG,

    MAX_SIG
};

#define MAX_PUB_SIG    (MAX_SIG + 1U)
#define QUEUESIZE      ((uint8_t)5)
#define POOLSIZE       ((uint8_t)5)

// Estrutura de um pacote LCP básico (cabeçalho)
#pragma pack(push, 1)
typedef struct {
    uint8_t  code;        // Código LCP: 0x01=Request, 0x02=Ack, 0x03=Nak, 0x04=Reject, 0x05=Term-Req, 0x06=Term-Ack
    uint8_t  identifier;  // Identificador para parear Request/Ack
    uint16_t length;      // Comprimento total do pacote (cabeçalho + opções)
    // aqui poderiam vir as opções…
} LcpPacket;
#pragma pack(pop)

// Evento genérico PPP (sem payload)
typedef struct {
    QEvt super;
} PPPEvt;

// Evento específico de LCP (carrega o identificador do pacote)
typedef struct {
    QEvt    super;
    uint8_t identifier;
} LCPEvt;

typedef struct {
    QEvt     super;
    uint8_t  identifier;
    uint16_t length;
    uint8_t  data[256];  // or allocate dynamically
} PayloadEvt;

void LCPEvt_init(LCPEvt * const me, QSignal sig, uint8_t identifier);

// Ativos externos
extern QActive * const AO_PPP;
extern void      PPP_ctor(void);

#endif // SINAIS_H_

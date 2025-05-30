#include "sinais.h"
#include "bsp.h"
#include <stdio.h>
#include <string.h>

/* Estrutura da máquina de estados PPP */
typedef struct {
	QActive super;
	int restart_counter;
	LcpPacket *packet;
} PPP;

static QState PPP_Initial(PPP *const me, QEvt const *const e);
void PPP_ctor(void);
/* Prototipação dos estados */
static QState Starting(PPP *const me, QEvt const *const e);
static QState AckReceived(PPP *const me, QEvt const *const e);
static QState AckSent(PPP *const me, QEvt const *const e);
static QState Opened(PPP *const me, QEvt const *const e);

/* Instância */
static PPP l_Tppp;
QActive *const AO_PPP = (QActive*) &l_Tppp;

int identifier = 0;
void PPP_ctor(void) {

	PPP *me = &l_Tppp;
	me->restart_counter = 0;

	QActive_ctor(&me->super, Q_STATE_CAST(&PPP_Initial));
}

static QState PPP_Initial(PPP *const me, QEvt const *const e) {

	QActive_subscribe((QActive*) me, RCR_PLUS_SIG);
	QActive_subscribe((QActive*) me, RCA_SIG);
	QActive_subscribe((QActive*) me, PL_SIG);
	QActive_subscribe((QActive*) me, PLA_SIG);
	send_configure_request();
	return Q_TRAN(&Starting);
}

static QState Starting(PPP *const me, QEvt const *const e) {
	switch (e->sig) {
	case Q_ENTRY_SIG: {
		return Q_HANDLED();
	}

	case RCR_PLUS_SIG: {
		LCPEvt const *lcpEvt = (LCPEvt const*) e;

		char answer[4];
		printf("Configure-Request [id=%u]. Aceitar Conexão? (s/n): ",
				lcpEvt->identifier);
		fflush(stdout);

		if (fgets(answer, sizeof(answer), stdin) != NULL
				&& (answer[0] == 's' || answer[0] == 'Y')) {
			send_configure_ack(lcpEvt->identifier);
			return Q_TRAN(&Opened);
		} else {
			printf("Conexão recusada\n");
			return Q_HANDLED();
		}
	}

	case RCA_SIG: {
		return Q_TRAN(&AckReceived);
	}

	default:
		return Q_SUPER(&QHsm_top);
	}
}

static QState AckReceived(PPP *const me, QEvt const *const e) {

	switch (e->sig) {
	case Q_ENTRY_SIG: {
		char userMsg[256];

		printf("Mensagem: ");
		fflush(stdout);

		if (fgets(userMsg, sizeof(userMsg), stdin) != NULL) {
			size_t len = strcspn(userMsg, "\r\n");
			userMsg[len] = '\0';

			sendMessage("%s", userMsg);
		} else {
			sendMessage("The quick brown fox jumps over the lazy dog!");
		}
		return Q_HANDLED();
	}
	case PLA_SIG: {
		// 1) Informa que a conexão foi encerrada
		printf("=== Conexão encerrada pelo peer ===\n");
		fflush(stdout);

		// 2) Pergunta ao usuário se quer iniciar de novo
		char answer[4];
		printf("Deseja iniciar uma nova conexão? (s/n): ");
		fflush(stdout);

		if (fgets(answer, sizeof(answer), stdin) != NULL
				&& (answer[0] == 's' || answer[0] == 'S')) {
			send_configure_request();
		} else {
			// usuário recusou: finaliza o AO (ou volta p/ Down, conforme seu design)
			printf("Encerrando aplicação...\n");
		}
		return Q_TRAN(&Starting);

	}
	}
	return Q_SUPER(&QHsm_top);
}

static QState Opened(PPP *const me, QEvt const *const e) {
	switch (e->sig) {
	case Q_ENTRY_SIG: {
		printf("== PPP Aberto ==\n");
		return Q_HANDLED();
	}
	case PL_SIG: {
		PayloadEvt const *pe = (PayloadEvt const*) e;

		// 1) Imprime o payload recebido
		fwrite(pe->data, 1, pe->length, stdout);
		printf("\n[PPP] Payload id=%u\n", pe->identifier);

		// 2) Envia o ACK
		messageAck(pe->identifier);

		// 3) Informa que a “sessão” de payload terminou e pergunta se quer reconectar
		printf("=== Sessão de dados encerrada ===\n");
		fflush(stdout);
		char answer[4];

		return Q_TRAN(&Starting);

	}

	}
	return Q_SUPER(&QHsm_top);
}

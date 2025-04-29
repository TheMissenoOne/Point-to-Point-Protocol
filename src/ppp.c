#include "sinais.h"
#include "bsp.h"
#include <stdio.h>

/* Estrutura da máquina de estados PPP */
typedef struct {
    QActive super;
    int restart_counter;
} PPP;

static QState PPP_Initial(PPP * const me, QEvt const * const e);
void PPP_ctor(void);
/* Prototipação dos estados */
static QState Starting(PPP * const me, QEvt const * const e);
static QState RequestSent(PPP * const me, QEvt const * const e);
static QState AckReceived(PPP * const me, QEvt const * const e);
static QState AckSent(PPP * const me, QEvt const * const e);
static QState Opened(PPP * const me, QEvt const * const e);

/* Instância */
static PPP l_Tppp;
QActive * const AO_PPP = (QActive *)&l_Tppp;

void PPP_ctor(void) {

	PPP *me = &l_Tppp;
	me->restart_counter = 0;

    QActive_ctor(&me->super, Q_STATE_CAST(&PPP_Initial));
}

static QState PPP_Initial(PPP * const me, QEvt const * const e)
{
	QActive_subscribe((QActive *)me, UP_SIG);
	QActive_subscribe((QActive *)me, DOWN_SIG);
	QActive_subscribe((QActive *)me, OPEN_SIG);
	QActive_subscribe((QActive *)me, CLOSE_SIG);
	QActive_subscribe((QActive *)me, RCR_PLUS_SIG);
	QActive_subscribe((QActive *)me, RCR_MINUS_SIG);
	QActive_subscribe((QActive *)me, RCA_SIG);
	QActive_subscribe((QActive *)me, RCN_SIG);
	QActive_subscribe((QActive *)me, RTR_SIG);
	QActive_subscribe((QActive *)me, RTA_SIG);
	QActive_subscribe((QActive *)me, TO_PLUS_SIG);
	QActive_subscribe((QActive *)me, TO_MINUS_SIG);

	return Q_TRAN(&Starting);
}

static QState Starting(PPP * const me, QEvt const * const e) {
    switch (e->sig) {
        case UP_SIG:
        	send_up();
            return Q_TRAN(&RequestSent);
        default:
            return Q_SUPER(&QHsm_top);
    }
}

static QState RequestSent(PPP * const me, QEvt const * const e) {
    switch (e->sig) {
    	case Q_ENTRY_SIG:
            send_configure_request();
            return Q_HANDLED();
        case RCR_PLUS_SIG:
            send_configure_ack();
            return Q_TRAN(&AckSent);
        case RCR_MINUS_SIG:
            // Ação: scn (Send-Configure-Nak ou Configure-Reject)
            //send_configure_nak();
            return Q_HANDLED();
        case RCA_SIG:
            return Q_TRAN(&AckReceived);
        case RCN_SIG:
            // Ação: scr (Send-Configure-Request)
            //send_configure_request();
            return Q_HANDLED();
        case TO_PLUS_SIG:
            if (me->restart_counter > 0) {
                me->restart_counter--;
               // send_configure_request();
                //start_restart_timer();
                return Q_HANDLED();
            } else {
                // Ação: tlf (This-Layer-Finished)
               // this_layer_finished();
                return Q_TRAN(&Starting);
            }
        default:
            return Q_SUPER(&QHsm_top);
    }
}

static QState AckSent(PPP * const me, QEvt const * const e) {
    switch (e->sig) {
        case RCA_SIG:
            return Q_TRAN(&Opened);
        case RCN_SIG:
            // Ação: scr (Send-Configure-Request)
           // send_configure_request();
            return Q_TRAN(&RequestSent);
        case TO_PLUS_SIG:
            if (me->restart_counter > 0) {
                me->restart_counter--;
                //send_configure_request();
                //start_restart_timer();
                return Q_HANDLED();
            } else {
                // Ação: tlf (This-Layer-Finished)
                //this_layer_finished();
                return Q_TRAN(&Starting);
            }
        default:
            return Q_SUPER(&QHsm_top);
    }
}

static QState AckReceived(PPP * const me, QEvt const * const e) {
    switch (e->sig) {
        case RCR_PLUS_SIG:
            // Ação: sca (Send-Configure-Ack)
           //send_configure_ack();
            return Q_TRAN(&Opened);
        case RCR_MINUS_SIG:
            // Ação: scn (Send-Configure-Nak ou Configure-Reject)
           // send_configure_nak();
            return Q_TRAN(&RequestSent);
        case TO_PLUS_SIG:
            if (me->restart_counter > 0) {
                me->restart_counter--;
               // send_configure_request();
               // start_restart_timer();
                return Q_TRAN(&RequestSent);
            } else {
                // Ação: tlf (This-Layer-Finished)
               // this_layer_finished();
                return Q_TRAN(&Starting);
            }
        default:
            return Q_SUPER(&QHsm_top);
    }
}

static QState Opened(PPP * const me, QEvt const * const e) {
    switch (e->sig) {
        case RCR_PLUS_SIG:
            // Ação: sca (Send-Configure-Ack), tld (This-Layer-Down)
            //send_configure_ack();
            //this_layer_down();
            return Q_TRAN(&AckSent);
        case RCR_MINUS_SIG:
            // Ação: scn (Send-Configure-Nak ou Configure-Reject), tld (This-Layer-Down)
            //send_configure_nak();
           // this_layer_down();
            return Q_TRAN(&RequestSent);
        case RTR_SIG:
            // Ação: sta (Send-Terminate-Ack), tld (This-Layer-Down)
           // send_terminate_ack();
           // this_layer_down();
            return Q_TRAN(&Starting);
        default:
            return Q_SUPER(&QHsm_top);
    }
}

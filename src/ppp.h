#ifndef PPP_H_
#define PPP_H_

#include <stdint.h>
#include "qp_port.h"
#include "sinais.h"    // para PppEvt, PPP_RX_FRAME_SIG

typedef struct PppEvtTag {
    QEvt super;
    uint8_t *frame;
    uint16_t length;
} PppEvt;

extern QActive * const AO_ppp;
void PPP_ctor(void);

// Construtor do evento dinâmico PppEvt para uso com Q_NEW:
// deve ter exatamente 4 parâmetros
void PppEvt_init(PppEvt * const me,
                 QSignal sig,
                 uint8_t *frame,
                 uint16_t length);

#endif // PPP_H_

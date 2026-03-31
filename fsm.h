#ifndef FSM_H
#define FSM_H

#include "types.h"

void fsm_init(Elevator *elevator);

void fsm_step(Elevator *elevator, int current_tick, int *total_wait, int *passengers_served);

#endif
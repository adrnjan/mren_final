#ifndef FSM_H
#define FSM_H

#include "types.h"

// Initializes the elevator to its starting state
void fsm_init(Elevator *elevator);

// Runs one "tick" or step of the simulation, updating the elevator's state 
// based on its current position and the priority queue.
void fsm_step(Elevator *elevator, int current_tick, int *total_wait, int *passengers_served);

#endif // FSM_H
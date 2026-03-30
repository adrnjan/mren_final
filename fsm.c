#include <stdio.h>
#include <stdbool.h>
#include "fsm.h"
#include "priority_queue.h"

// We store the current request the elevator is actively trying to fulfill.
static Request active_request;
static bool has_active_request = false;

void fsm_init(Elevator *elevator) {
    elevator->current_floor = 1; // Start on the ground floor
    elevator->current_state = STATE_IDLE;
    elevator->doors_open = true; // Doors open by default when idle at the bottom
    has_active_request = false;
    printf("[FSM] Elevator initialized at floor 1. Status: IDLE, Doors: OPEN\n");
}

void fsm_step(Elevator *elevator, int current_tick, int *total_wait, int *passengers_served){
    // A classic switch statement handles the Finite State Machine transitions
    switch (elevator->current_state) {

        case STATE_IDLE:
            if (!has_active_request) {
                // If we don't have a target, check the priority queue for the most important request
                if (!pq_is_empty()) {
                    pq_extract(&active_request);
                    has_active_request = true;
                    printf("[FSM] Extracted new request: Target Floor %d (Priority: %d)\n", 
                           active_request.target_floor, active_request.priority_score);
                }
            }

            // If we now have a target, decide which way to move
            if (has_active_request) {
                if (elevator->doors_open) {
                    printf("[FSM] Closing doors...\n");
                    elevator->doors_open = false;
                }

                if (active_request.target_floor > elevator->current_floor) {
                    elevator->current_state = STATE_MOVING_UP;
                    printf("[FSM] Transition -> MOVING UP\n");
                } 
                else if (active_request.target_floor < elevator->current_floor) {
                    elevator->current_state = STATE_MOVING_DOWN;
                    printf("[FSM] Transition -> MOVING DOWN\n");
                } 
                else {
                    // We are already on the requested floor!
                    elevator->current_state = STATE_DOORS_OPEN;
                    printf("[FSM] Transition -> DOORS OPEN (Already on target floor)\n");
                }
            }
            break;

        case STATE_MOVING_UP:
                elevator->current_floor++;
                printf("[FSM] Elevator moving up... now at floor %d\n", elevator->current_floor);

                int wait_time_down = 0;
                if (elevator->current_floor != active_request.target_floor &&
                    pq_check_and_remove_target(elevator->current_floor, current_tick, &wait_time_down)) {
                    printf("[FSM] En-route stop at floor %d (continuing to floor %d). Wait time: %d ticks\n",
                    elevator->current_floor, active_request.target_floor, wait_time_down);
                    *total_wait += wait_time_down;
                    (*passengers_served)++;
                    pq_insert(active_request.target_floor, elevator->current_floor, elevator->current_state, current_tick);
                    elevator->current_state = STATE_DOORS_OPEN;
                    break;
                }
    if (elevator->current_floor == active_request.target_floor) {
        printf("[FSM] Arrived at target floor %d!\n", elevator->current_floor);
        elevator->current_state = STATE_DOORS_OPEN;
        printf("[FSM] Transition -> DOORS OPEN\n");
    }
    break;

        case STATE_MOVING_DOWN:
            elevator->current_floor--;
            printf("[FSM] Elevator moving down... now at floor %d\n", elevator->current_floor);

            int wait_time = 0;
            if (elevator->current_floor != active_request.target_floor &&
                pq_check_and_remove_target(elevator->current_floor, current_tick, &wait_time)) {
                printf("[FSM] En-route stop at floor %d (continuing to floor %d). Wait time: %d ticks\n",
                elevator->current_floor, active_request.target_floor, wait_time);
                *total_wait += wait_time;
                (*passengers_served)++;
                pq_insert(active_request.target_floor, elevator->current_floor, elevator->current_state, current_tick);
                elevator->current_state = STATE_DOORS_OPEN;
             break;
    }
    if (elevator->current_floor == active_request.target_floor) {
        printf("[FSM] Arrived at target floor %d!\n", elevator->current_floor);
        elevator->current_state = STATE_DOORS_OPEN;
        printf("[FSM] Transition -> DOORS OPEN\n");
    }
    break;

        case STATE_DOORS_OPEN:
    {
        int wait = current_tick - active_request.request_tick;
        *total_wait += wait;
        (*passengers_served)++;
        printf("[FSM] Doors open at floor %d. Passenger wait time: %d ticks\n",
               elevator->current_floor, wait);
        elevator->doors_open = true;
        has_active_request = false;
        elevator->current_state = STATE_IDLE;
        printf("[FSM] Transition -> IDLE\n");
    }
    break;
    }
}
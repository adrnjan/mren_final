#include <stdio.h>
#include <stdbool.h>
#include "fsm.h"
#include "priority_queue.h"

static Request active_request;
static bool has_active_request = false;

static bool en_route_stop = false;

void fsm_init(Elevator *elevator) {
    elevator->current_floor = 1;
    elevator->current_state = STATE_IDLE;
    elevator->doors_open = true;
    has_active_request = false;
    en_route_stop = false;
    printf("[FSM] Elevator initialized at floor 1. Status: IDLE, Doors: OPEN\n");
}

void fsm_step(Elevator *elevator, int current_tick, int *total_wait, int *passengers_served){
    switch (elevator->current_state) {

        case STATE_IDLE:
            if (!has_active_request) {
                if (!pq_is_empty()) {
                    pq_refresh_priorities(elevator->current_floor, elevator->current_state, current_tick);
                    pq_extract(&active_request);
                    has_active_request = true;
                    printf("[FSM] Extracted new request: Target Floor %d (Priority: %d)\n", 
                           active_request.target_floor, active_request.priority_score);
                }
            }

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
                    elevator->current_state = STATE_DOORS_OPEN;
                    printf("[FSM] Transition -> DOORS OPEN (Already on target floor)\n");
                }
            }
            break;

        case STATE_MOVING_UP:
            elevator->current_floor++;
            printf("[FSM] Elevator moving up... now at floor %d\n", elevator->current_floor);

            int wait_time_up = 0;
            if (elevator->current_floor != active_request.target_floor &&
                pq_size() < MAX_QUEUE_SIZE &&
                pq_check_and_remove_target(elevator->current_floor, current_tick, &wait_time_up)) {
                printf("[FSM] En-route stop at floor %d (continuing to floor %d). Passenger wait time: %d ticks\n",
                       elevator->current_floor, active_request.target_floor, wait_time_up);
                *total_wait += wait_time_up;
                (*passengers_served)++;
                pq_insert(active_request.target_floor, elevator->current_floor, elevator->current_state, active_request.request_tick);
                en_route_stop = true;
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

            int wait_time_down = 0;
            if (elevator->current_floor != active_request.target_floor &&
                pq_size() < MAX_QUEUE_SIZE &&
                pq_check_and_remove_target(elevator->current_floor, current_tick, &wait_time_down)) {
                printf("[FSM] En-route stop at floor %d (continuing to floor %d). Passenger wait time: %d ticks\n",
                       elevator->current_floor, active_request.target_floor, wait_time_down);
                *total_wait += wait_time_down;
                (*passengers_served)++;
                pq_insert(active_request.target_floor, elevator->current_floor, elevator->current_state, active_request.request_tick);
                en_route_stop = true;
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
            if (en_route_stop) {
                printf("[FSM] Doors open at floor %d (en-route stop).\n", elevator->current_floor);
                en_route_stop = false;
            } else {
                int wait = current_tick - active_request.request_tick;
                *total_wait += wait;
                (*passengers_served)++;
                printf("[FSM] Doors open at floor %d. Passenger wait time: %d ticks\n",
                       elevator->current_floor, wait);
            }
            elevator->doors_open = true;
            has_active_request = false;
            elevator->current_state = STATE_IDLE;
            printf("[FSM] Transition -> IDLE\n");
        }
        break;
    }
}
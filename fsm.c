#include <stdio.h>
#include <stdbool.h>
#include "fsm.h"
#include "priority_queue.h"

static Request active_request;
static bool has_active_request = false;

static bool en_route_stop = false;

void fsm_init(Elevator *elevator) { //initialize with default values
    elevator->current_floor = 1;
    elevator->current_state = STATE_IDLE;
    elevator->doors_open = true;
    has_active_request = false;
    en_route_stop = false;
    printf("[FSM] Elevator initialized at floor 1. Status: Idle, Doors: Open\n");
}

void fsm_step(Elevator *elevator, int current_tick, int *total_wait, int *passengers_served){
    switch (elevator->current_state) {

        case STATE_IDLE:
            if (!has_active_request) { //if no active request check if there are any requests in the priority queue
                if (!pq_is_empty()) { //if there are requests, refresh priorities, extract the highest priority request, set has_active_request to true, and print the extracted request
                    pq_refresh_priorities(elevator->current_floor, elevator->current_state, current_tick);
                    pq_extract(&active_request);
                    has_active_request = true;
                    printf("[FSM] Extracted new request: Target Floor %d (Priority: %d)\n", 
                           active_request.target_floor, active_request.priority_score);
                }
            }

            if (has_active_request) {
                if (elevator->doors_open) { //"close" the doors (no fsm for closed doors, just a bool)
                    printf("[FSM] Closing doors...\n");
                    elevator->doors_open = false;
                }

                if (active_request.target_floor > elevator->current_floor) { //if target floor is above current floor, move up
                    elevator->current_state = STATE_MOVING_UP;
                    printf("[FSM] Transition -> Moving Up\n");
                } 
                else if (active_request.target_floor < elevator->current_floor) { //if target floor is below current floor, move down
                    elevator->current_state = STATE_MOVING_DOWN;
                    printf("[FSM] Transition -> Moving Down\n");
                } 
                else { //if target floor is same as current floor, open doors
                    elevator->current_state = STATE_DOORS_OPEN;
                    printf("[FSM] Transition -> Doors Open (Already on target floor)\n");
                }
            }
            break;

        case STATE_MOVING_UP:
            elevator->current_floor++;
            printf("[FSM] Elevator moving up... now at floor %d\n", elevator->current_floor);

            int wait_time_up = 0;
            if (elevator->current_floor != active_request.target_floor &&
                pq_size() < MAX_QUEUE_SIZE &&
                pq_check_and_remove_target(elevator->current_floor, current_tick, &wait_time_up)) { //enroute case(not @detination), wont stop unless there is guarantee the re insertion will succeed, so we check if room in queue.
                printf("[FSM] En-route stop at floor %d (continuing to floor %d). Passenger wait time: %d ticks\n",
                       elevator->current_floor, active_request.target_floor, wait_time_up); //we passed wait time up to calc how long requested existed, so we now print that
                *total_wait += wait_time_up; //add to total for average calc after
                (*passengers_served)++; 
                pq_insert(active_request.target_floor, elevator->current_floor, elevator->current_state, active_request.request_tick); //reinsert original target request to queue(might be at top, not necessarily, depends on priority and maybe new request was made)
                en_route_stop = true; //set en_route_stop to true for preventing counting passenger twice(once during enroute and again in doors open)
                elevator->current_state = STATE_DOORS_OPEN; //for next call
                break;
            }
            if (elevator->current_floor == active_request.target_floor) { //if we are at target, then simply change to state doors open for next call
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
                       elevator->current_floor, active_request.target_floor, wait_time_down); //enroute case, same logic as up
                *total_wait += wait_time_down;
                (*passengers_served)++;
                pq_insert(active_request.target_floor, elevator->current_floor, elevator->current_state, active_request.request_tick); //reinsert original target request to queue(might be at top, not necessarily, depends on priority and maybe new request was made)
                en_route_stop = true; //set en_route_stop to true for preventing counting passenger twice(once during enroute and again in doors open)
                elevator->current_state = STATE_DOORS_OPEN; //for next call
                break;
            }
            if (elevator->current_floor == active_request.target_floor) { //if we are at target, then simply change to state doors open for next call
                printf("[FSM] Arrived at target floor %d!\n", elevator->current_floor);
                elevator->current_state = STATE_DOORS_OPEN;
                printf("[FSM] Transition -> DOORS OPEN\n");
            }
            break;

        case STATE_DOORS_OPEN:
        {
            if (en_route_stop) { //here we prevent double counting en-route passengers
                printf("[FSM] Doors open at floor %d (en-route stop).\n", elevator->current_floor);
                en_route_stop = false;
            } else { //normal case, we count passenger served here and calc their wait time
                int wait = current_tick - active_request.request_tick;
                *total_wait += wait;
                (*passengers_served)++;
                printf("[FSM] Doors open at floor %d. Passenger wait time: %d ticks\n",
                       elevator->current_floor, wait);
            }
            elevator->doors_open = true; //this is just for simulation output to pring doors closing and opening, not really functional
            has_active_request = false; //set has_active_request to false so that in next call we extract a new request
            elevator->current_state = STATE_IDLE; //set state to idle for next call
            printf("[FSM] Transition -> IDLE\n");
        }
        break;
    }
}
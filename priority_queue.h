#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdbool.h>
#include "types.h"

void pq_init(void);

bool pq_is_empty(void);

int calculate_priority(int target_floor, int current_floor, ElevatorState current_state, int wait_so_far);

bool pq_insert(int target_floor, int current_floor, ElevatorState current_state, int current_tick);

bool pq_extract(Request *out_request);

bool pq_check_and_remove_target(int floor, int current_tick, int *wait_time_out);

void pq_refresh_priorities(int current_floor, ElevatorState current_state, int current_tick);

int pq_size(void);

#endif
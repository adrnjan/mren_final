#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdbool.h>
#include "types.h"

// Initializes/resets the priority queue
void pq_init(void);

// Checks if the queue is empty
bool pq_is_empty(void);

// Calculate the priority score
int calculate_priority(int target_floor, int current_floor, ElevatorState current_state);

// Inserts a new floor request into the binary heap
bool pq_insert(int target_floor, int current_floor, ElevatorState current_state, int current_tick);

// Extracts the request with the highest priority score
bool pq_extract(Request *out_request);

bool pq_check_and_remove_target(int floor, int current_tick, int *wait_time_out);

#endif
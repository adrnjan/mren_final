#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#define NUM_FLOORS 10
#define MAX_QUEUE_SIZE 50

typedef enum {
    STATE_IDLE,
    STATE_MOVING_UP,
    STATE_MOVING_DOWN,
    STATE_DOORS_OPEN
} ElevatorState;


typedef struct {
    int target_floor;
    int priority_score;
    int request_tick; 
} Request;


typedef struct {
    int current_floor;
    ElevatorState current_state;
    bool doors_open;
} Elevator;

#endif
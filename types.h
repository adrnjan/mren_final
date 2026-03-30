#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

//System Constraints
#define NUM_FLOORS 10
#define MAX_QUEUE_SIZE 50

//FSM Definitions
typedef enum {
    STATE_IDLE,
    STATE_MOVING_UP,
    STATE_MOVING_DOWN,
    STATE_DOORS_OPEN
} ElevatorState;

// Represents a single floor request both internal/external
typedef struct {
    int target_floor;
    int priority_score;
    int request_tick; 
} Request;

// Represents the real-time physical state of the elevator car.
typedef struct {
    int current_floor;
    ElevatorState current_state;
    bool doors_open;
} Elevator;

#endif
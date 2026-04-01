#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#define NUM_FLOORS 10 //10 floor building
#define MAX_QUEUE_SIZE 50 //queue can hold a max of 50 requests

typedef enum {
    STATE_IDLE,
    STATE_MOVING_UP,
    STATE_MOVING_DOWN,
    STATE_DOORS_OPEN
} ElevatorState;


typedef struct {
    int target_floor; //floor the request is for
    int priority_score; //priority of the request
    int request_tick; //tick the request was made
} Request;


typedef struct {
    int current_floor; //what floor elevator is currently
    ElevatorState current_state; //the state of whether its moving down/up or idle or door open
    bool doors_open; //bool if the doors are open or not
} Elevator;

#endif
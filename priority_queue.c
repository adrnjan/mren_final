#include <stdio.h>
#include <stdlib.h>
#include "priority_queue.h"

static Request heap[MAX_QUEUE_SIZE];
static int heap_size = 0;

static int floor_count[NUM_FLOORS + 1];

void pq_init(void) {
    heap_size = 0;
    for (int i = 0; i <= NUM_FLOORS; i++) {
        floor_count[i] = 0;
    }
}

bool pq_is_empty(void) {
    return (heap_size == 0);
}

static void swap(Request *a, Request *b) {
    Request temp = *a;
    *a = *b;
    *b = temp;
}

int calculate_priority(int target_floor, int current_floor, ElevatorState current_state, int wait_so_far) {
    int score = 0;
    int distance = abs(target_floor - current_floor);

    if (current_state == STATE_IDLE) {
        score = 100 - distance; 
    } 
    else if (current_state == STATE_MOVING_UP) {
        if (target_floor > current_floor) {
            score = 200 - distance; 
        } else {
            score = 50 - distance;  
        }
    } 
    else if (current_state == STATE_MOVING_DOWN) {
        if (target_floor < current_floor) {
            score = 200 - distance; 
        } else {
            score = 50 - distance;  
        }
    }

    score += wait_so_far * 2;

    return score;
}

static void heapify_up(int index) {
    if (index == 0) return; 

    int parent_index = (index - 1) / 2;

    if (heap[index].priority_score > heap[parent_index].priority_score) {
        swap(&heap[index], &heap[parent_index]);
        heapify_up(parent_index); 
    }
}

bool pq_insert(int target_floor, int current_floor, ElevatorState current_state, int current_tick) {
    if (heap_size >= MAX_QUEUE_SIZE) {
        printf("Error: Queue is full. Cannot accept request for floor %d.\n", target_floor);
        return false;
    }
    Request new_req;
    new_req.target_floor = target_floor;
    new_req.priority_score = calculate_priority(target_floor, current_floor, current_state, 0);
    new_req.request_tick = current_tick;
    heap[heap_size] = new_req;
    heapify_up(heap_size);
    heap_size++;

    if (target_floor >= 1 && target_floor <= NUM_FLOORS) {
        floor_count[target_floor]++;
    }

    return true;
}

static void heapify_down(int index) {
    int largest = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < heap_size && heap[left_child].priority_score > heap[largest].priority_score) {
        largest = left_child;
    }

    if (right_child < heap_size && heap[right_child].priority_score > heap[largest].priority_score) {
        largest = right_child;
    }

    if (largest != index) {
        swap(&heap[index], &heap[largest]);
        heapify_down(largest);
    }
}

bool pq_extract(Request *out_request) {
    if (pq_is_empty()) {
        return false;
    }

    *out_request = heap[0];

    if (out_request->target_floor >= 1 && out_request->target_floor <= NUM_FLOORS) {
        floor_count[out_request->target_floor]--;
    }

    heap[0] = heap[heap_size - 1];
    heap_size--;

    heapify_down(0);

    return true;
}

bool pq_check_and_remove_target(int floor, int current_tick, int *wait_time_out) {
    if (floor >= 1 && floor <= NUM_FLOORS && floor_count[floor] == 0) {
        return false;
    }

    for (int i = 0; i < heap_size; i++) {
        if (heap[i].target_floor == floor) {
            *wait_time_out = current_tick - heap[i].request_tick;

            heap[i] = heap[heap_size - 1];
            heap_size--;

            if (heap_size > 0 && i < heap_size) {
                heapify_down(i);
                heapify_up(i);
            }

            if (floor >= 1 && floor <= NUM_FLOORS) {
                floor_count[floor]--;
            }

            printf("[PQ] Floor %d removed from queue (serviced en route)\n", floor);
            if (floor >= 1 && floor <= NUM_FLOORS && floor_count[floor] > 0) {
                printf("[PQ] Note: %d additional request(s) for floor %d still in queue\n",
                       floor_count[floor], floor);
            }
            return true;
        }
    }
    return false;
}

void pq_refresh_priorities(int current_floor, ElevatorState current_state, int current_tick) {
    for (int i = 0; i < heap_size; i++) {
        int wait_so_far = current_tick - heap[i].request_tick;
        heap[i].priority_score = calculate_priority(
            heap[i].target_floor, current_floor, current_state, wait_so_far);
    }
    for (int i = heap_size / 2 - 1; i >= 0; i--) {
        heapify_down(i);
    }
}

int pq_size(void) {
    return heap_size;
}
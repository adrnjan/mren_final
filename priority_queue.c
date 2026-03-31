#include <stdio.h>
#include <stdlib.h>
#include "priority_queue.h"

// array representing binary max-heap
static Request heap[MAX_QUEUE_SIZE];
static int heap_size = 0;

void pq_init(void) {
    heap_size = 0;
}

bool pq_is_empty(void) {
    return (heap_size == 0);
}

// Helper function to swap two requests in the heap
static void swap(Request *a, Request *b) {
    Request temp = *a;
    *a = *b;
    *b = temp;
}

// Calculates a dynamic priority score based on elevator's current physical state.
int calculate_priority(int target_floor, int current_floor, ElevatorState current_state) {
    int score = 0;
    int distance = abs(target_floor - current_floor);

    if (current_state == STATE_IDLE) {
        // prioritize the closest floor
        score = 100 - distance; 
    } 
    else if (current_state == STATE_MOVING_UP) {
        if (target_floor > current_floor) {
            // on the way score
            score = 200 - distance; 
        } else {
            // not on the way score
            score = 50 - distance;  
        }
    } 
    else if (current_state == STATE_MOVING_DOWN) {
        if (target_floor < current_floor) {
            // on the way score
            score = 200 - distance; 
        } else {
            // not on the way score
            score = 50 - distance;  
        }
    }
    return score;
}

// Maintains the max-heap property after insertion
static void heapify_up(int index) {
    if (index == 0) return; // Reached the root

    int parent_index = (index - 1) / 2;

    // If the current node has a higher priority than its parent, swap them
    if (heap[index].priority_score > heap[parent_index].priority_score) {
        swap(&heap[index], &heap[parent_index]);
        heapify_up(parent_index); // Recursion upwards
    }
}

bool pq_insert(int target_floor, int current_floor, ElevatorState current_state, int current_tick) {
    if (heap_size >= MAX_QUEUE_SIZE) {
        printf("Error: Queue is full. Cannot accept request for floor %d.\n", target_floor);
        return false;
    }

    // Create the new request and assigns it a score
    Request new_req;
    new_req.target_floor = target_floor;
    new_req.priority_score = calculate_priority(target_floor, current_floor, current_state);

        new_req.request_tick = current_tick;
        heap[heap_size] = new_req;
        heapify_up(heap_size);
        heap_size++;

    return true;
}

// Maintains the max-heap property after extraction
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

    // If the largest is not the current node, swap and recurse downwards
    if (largest != index) {
        swap(&heap[index], &heap[largest]);
        heapify_down(largest);
    }
}

bool pq_extract(Request *out_request) {
    if (pq_is_empty()) {
        return false;
    }

    // The highest priority request is always at the root aka index 0
    *out_request = heap[0];

    // Move the last element to the root and reduce size
    heap[0] = heap[heap_size - 1];
    heap_size--;

    // bring the new root down to its correct position
    heapify_down(0);

    return true;
}

bool pq_check_and_remove_target(int floor, int current_tick, int *wait_time_out) {
    for (int i = 0; i < heap_size; i++) {
        if (heap[i].target_floor == floor) {
            *wait_time_out = current_tick - heap[i].request_tick;
            heap[i] = heap[heap_size - 1];
            heap_size--;
            if (heap_size > 0 && i < heap_size) {
                heapify_down(i);
            }
            printf("[PQ] Floor %d removed from queue (serviced en route)\n", floor);
            return true;
        }
    }
    return false;
}
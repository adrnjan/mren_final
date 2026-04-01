#include <stdio.h>
#include <stdlib.h>
#include "priority_queue.h"
//max heap structure: complete binary tree where the parent node is greater than or equal to all of its children. o(1) for find max element, o(log n) for insert and delete
//used since efficient for removing highest value(most priority score)o(log n)
static Request heap[MAX_QUEUE_SIZE]; //array that holds requests
static int heap_size = 0;//size of heap

static int floor_count[NUM_FLOORS + 1];//array of number of requests for each floor

void pq_init(void) { //initializes the priority queue,set num commands of each floor to 0 at first
    heap_size = 0;
    for (int i = 0; i <= NUM_FLOORS; i++) {
        floor_count[i] = 0;
    }
}

bool pq_is_empty(void) {//checks if queue is empty
    return (heap_size == 0);
}

static void swap(Request *a, Request *b) {//swaps two request(helper function) used when entire request structs need to be swapped due to new command having higher priority score for example
    Request temp = *a;
    *a = *b;
    *b = temp;
}

int calculate_priority(int target_floor, int current_floor, ElevatorState current_state, int wait_so_far) {
    int score = 0;
    int distance = abs(target_floor - current_floor);

    if (current_state == STATE_IDLE) { //if elevator is idle, priority is based on distance from current floor
        score = 100 - distance; 
    } 
    else if (current_state == STATE_MOVING_UP) { //when elevator travel up, will prioritze request on the way going up, if not will give lower priority
        if (target_floor > current_floor) {
            score = 200 - distance; 
        } else {
            score = 50 - distance;  
        }
    } 
    else if (current_state == STATE_MOVING_DOWN) { //when elevator travel down, will prioritze request on the way going down, if not will give lower
        if (target_floor < current_floor) {
            score = 200 - distance; 
        } else {
            score = 50 - distance; 
        }
    }

    score += wait_so_far * 2; //add weight for how long request has been waiting(aging function), mainly for large scale commands. after 75 ticks it overcomes 150 same direction advantage

    return score;
}

static void heapify_up(int index) {//compares element with its parent, swaps if bigger and repeats, keeps moving up until heap property is restored(root is max)(recursive function)
    if (index == 0) return; 

    int parent_index = (index - 1) / 2; //find parent index (using the formula to find parent at any index for binary heap)

    if (heap[index].priority_score > heap[parent_index].priority_score) { //if current element is bigger than parent, swap them
        swap(&heap[index], &heap[parent_index]);
        heapify_up(parent_index); //recursive call to continue going up until heap property is restored
    }
}

bool pq_insert(int target_floor, int current_floor, ElevatorState current_state, int current_tick) { //inserts a new request into the heap at the end, uses heapify up to restore max heap property. O(log n)
    if (heap_size >= MAX_QUEUE_SIZE) { //checks if queue is full, if so cannot insert
        printf("Error: Queue is full. Cannot accept request for floor %d.\n", target_floor);
        return false;
    }
    Request new_req; //creates new request struct and assigns values that were read from the input file
    new_req.target_floor = target_floor;
    new_req.priority_score = calculate_priority(target_floor, current_floor, current_state, 0); 
    new_req.request_tick = current_tick;
    heap[heap_size] = new_req; 
    heapify_up(heap_size);
    heap_size++; 

    if (target_floor >= 1 && target_floor <= NUM_FLOORS) {//updates the number of requests for the target floor
        floor_count[target_floor]++;
    }

    return true;
}

static void heapify_down(int index) { //compares element with its children, if smaller, then swap with the larger child and keep recursiveley going, until heap property is restored(root is max)(recursive function)
    int largest = index;
    int left_child = 2 * index + 1; //these two use formula to locate left right child by index
    int right_child = 2 * index + 2;

    if (left_child < heap_size && heap[left_child].priority_score > heap[largest].priority_score) { //if left child is bigger than parent, swap them
        largest = left_child;
    }

    if (right_child < heap_size && heap[right_child].priority_score > heap[largest].priority_score) { //if right child is bigger than parent, swap them
        largest = right_child;
    }

    if (largest != index) { //after both comparisons, if largest is not the parent, swap them and recursively call heapify_down on the largest child, otherwise job is done.
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
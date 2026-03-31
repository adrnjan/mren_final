#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "types.h"
#include "priority_queue.h"
#include "fsm.h"

typedef struct {
    int tick;
    int target_floor;
} FileCommand;

int main() {
    printf("------------------------------------------------------\n");
    printf("   Intelligent Elevator Dispatch System   \n");
    printf("-------------------------------------------------------\n\n");

    FILE *file = fopen("scenario.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open scenario.txt. Either corrupt or missing.\n");
        return 1;
    }

    FileCommand commands[50];
    int cmd_count = 0;
    int temp_tick, temp_floor;
    char line[100];
    int line_number = 0;
    while (cmd_count < 50 && fgets(line, sizeof(line), file)) {
        line_number++;
        if (sscanf(line, "%d %d", &temp_tick, &temp_floor) != 2) {
            printf("[WARNING] Ignoring malformed line %d in scenario.txt: %s", line_number, line);
            continue;
        }
        if (temp_floor < 1 || temp_floor > NUM_FLOORS) {
            printf("[WARNING] Ignoring invalid floor %d on line %d (valid range: 1-%d)\n", temp_floor, line_number, NUM_FLOORS);
            continue;
        }
        if (temp_tick < 1) {
            printf("[WARNING] Ignoring invalid tick %d on line %d (must be >= 1)\n", temp_tick, line_number);
            continue;
        }
        commands[cmd_count].tick = temp_tick;
        commands[cmd_count].target_floor = temp_floor;
        cmd_count++;
    }
    if (cmd_count >= 50 && fgets(line, sizeof(line), file)) {
        printf("[WARNING] scenario.txt has more than 50 commands. Only the first 50 were loaded.\n");
    }
    fclose(file);
    printf("Loaded %d commands from scenario.txt\n", cmd_count);

    if (cmd_count == 0) {
        printf("[WARNING] No valid commands loaded. Simulation will run idle.\n");
    }

    int max_tick = 0;
    for (int i = 0; i < cmd_count; i++)
        if (commands[i].tick > max_tick) max_tick = commands[i].tick;
    const int SIMULATION_LENGTH = max_tick + 50;

    Elevator my_elevator;
    pq_init();
    fsm_init(&my_elevator);

    int total_wait = 0;
    int passengers_served = 0;
    int total_floors_traveled = 0;
    int previous_floor = my_elevator.current_floor;

    printf("\n--- Starting Simulation ---\n\n");

    for (int tick = 1; tick <= SIMULATION_LENGTH; tick++) {
        printf("\n[Time: Tick %d]\n", tick);

        for (int i = 0; i < cmd_count; i++) {
            if (commands[i].tick == tick) {
                printf("[INPUT] Request for Floor %d\n", commands[i].target_floor);
                if (!pq_insert(commands[i].target_floor, my_elevator.current_floor,
                               my_elevator.current_state, tick)) {
                    printf("[WARNING] Could not enqueue request for floor %d (queue full)\n",
                           commands[i].target_floor);
                }
            }
        }

        fsm_step(&my_elevator, tick, &total_wait, &passengers_served);

        if (my_elevator.current_floor != previous_floor) {
            total_floors_traveled++;
            previous_floor = my_elevator.current_floor;
        }
    }

    printf("\n------------------------------------------------\n");
    printf("               Simulation Complete!               \n");
    printf("------------------------------------------------\n");
    printf("Final State: %s\n", (my_elevator.current_state == STATE_IDLE) ? "IDLE" : "MOVING/BUSY");
    printf("Final Floor: %d\n", my_elevator.current_floor);
    printf("Total Floors Traveled: %d\n", total_floors_traveled);
    printf("Passengers Served: %d\n", passengers_served);
    if (passengers_served > 0)
        printf("Average Wait Time: %.1f ticks\n", (float)total_wait / passengers_served);
    printf("--------------------------------------------------\n");

    return 0;
}

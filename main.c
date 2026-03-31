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

    // file reading
    FILE *file = fopen("scenario.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open scenario.txt. Either corrupt or missing.\n");
        return 1;
    }

    FileCommand commands[50]; // max commands is 50
    int cmd_count = 0;
    while (fscanf(file, "%d %d", &commands[cmd_count].tick,
                  &commands[cmd_count].target_floor) == 2) {
        cmd_count++;
    }
    fclose(file);
    printf("Loaded %d simulations from scenario.txt\n", cmd_count);


    int max_tick = 0;
    for (int i = 0; i < cmd_count; i++)
        if (commands[i].tick > max_tick) max_tick = commands[i].tick;
    const int SIMULATION_LENGTH = max_tick + 15;

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
                pq_insert(commands[i].target_floor, my_elevator.current_floor,
                          my_elevator.current_state, tick);
            }
        }

        fsm_step(&my_elevator, tick, &total_wait, &passengers_served);

        if (my_elevator.current_floor != previous_floor) {
            total_floors_traveled++;
            previous_floor = my_elevator.current_floor;
        }
    }

    // 5. Final metrics
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

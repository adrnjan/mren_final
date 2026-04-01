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

    FILE *file = fopen("scenario.txt", "r"); //opens txt file with fopen, checks if it doesnt exist, r just means read only
    if (file == NULL) {
        printf("Error: Could not open scenario.txt. Either corrupt or missing.\n");
        return 1;
    }

    FileCommand commands[50]; //array that holds commands (up to 50)
    int cmd_count = 0; //counter for number of commands
    int temp_tick, temp_floor; //temp variables to hold the tick and floor
    char line[100]; //buffer for each line (basically one row of line as we parse each line)
    int line_number = 0; //line number counter
    while (cmd_count < 50 && fgets(line, sizeof(line), file)) { //parses the file for commands(basically reads up to 50 rows of the txt file), fgets returns null if read error or end of file
        line_number++;
        if (sscanf(line, "%d %d", &temp_tick, &temp_floor) != 2) { //sscanf checks to see if the line is inputted valid(ignores something like "1 abc")
            printf("[WARNING] Ignoring invalid line %d in scenario.txt: %s", line_number, line);
            continue;
        }
        if (temp_floor < 1 || temp_floor > NUM_FLOORS) { //checks to see if the inputted floor is within the range of 1 to 10
            printf("[WARNING] Ignoring invalid floor %d on line %d (valid range: 1-%d)\n", temp_floor, line_number, NUM_FLOORS);
            continue;
        }
        if (temp_tick < 1) { //checks to see if the inputted tick is 1 or above
            printf("[WARNING] Ignoring invalid tick %d on line %d (must be >= 1)\n", temp_tick, line_number);
            continue;
        }
        commands[cmd_count].tick = temp_tick; //sets the tick of the current command to be completed
        commands[cmd_count].target_floor = temp_floor; //sets the floor of the current command to be completed
        cmd_count++; //increments the command counter
    }
    if (cmd_count >= 50 && fgets(line, sizeof(line), file)) { //checks if there are more than 50 commands
        printf("[WARNING] scenario.txt has more than 50 commands. Only the first 50 are processed.\n");
    }
    fclose(file);
    printf("Loaded %d commands from scenario.txt\n", cmd_count); //prints the number of commands loaded

    if (cmd_count == 0) { //checks if no valid commands
        printf("[WARNING] No valid commands loaded. Simulation will run idle.\n");
    }

    int max_tick = 0; //finds the max tick to find out how long to run simulation(adds buffer to ensure all commands finish)
    for (int i = 0; i < cmd_count; i++)
        if (commands[i].tick > max_tick) max_tick = commands[i].tick;
    const int SIMULATION_LENGTH = max_tick + 50; 

    Elevator my_elevator; //creates elevator struct
    pq_init(); //priority queue
    fsm_init(&my_elevator); //finite state machine

    int total_wait = 0; //total wait time
    int passengers_served = 0; //passengers served
    int total_floors_traveled = 0; //total floors traveled
    int previous_floor = my_elevator.current_floor; //previous floor

    printf("\n--- Starting Simulation ---\n\n");

    for (int tick = 1; tick <= SIMULATION_LENGTH; tick++) { //runs through full tick length
        printf("\n[Time: Tick %d]\n", tick);

        for (int i = 0; i < cmd_count; i++) { //checks if there a request at each tick
            if (commands[i].tick == tick) { //basically checks if the current tick is equal to the tick of the command
                printf("[INPUT] Request for Floor %d\n", commands[i].target_floor);
                if (!pq_insert(commands[i].target_floor, my_elevator.current_floor,
                               my_elevator.current_state, tick)) { //inserts request into priority queue and also then checks if queue is full(past 50 requests, moreso a safety check, we already limited to 50, but there in case commands[changed here])
                    printf("[WARNING] Could not enqueue request for floor %d (queue full)\n",
                           commands[i].target_floor);
                }
            }
        }

        fsm_step(&my_elevator, tick, &total_wait, &passengers_served); //fsm will modify myelevator state to transition between states, tick will be used to calc wait tiem, total wait, and also increment passenger served everytime door opens.

        if (my_elevator.current_floor != previous_floor) { //checks if the elevator moved
            total_floors_traveled++; //increments total floors traveled
            previous_floor = my_elevator.current_floor; //updates previous floor
        }
    }
    //after the simulation is complete, results are printed
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

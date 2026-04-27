#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "parking.h"
#include "globals.h"
#include "helper.h"

void init_parking(ParkingLot *lot, int intersection_id) {
    if (lot == NULL) {
        safe_print("Error no parking lot provided\n");
        return;
    }
    lot->id = intersection_id; // Set the parking lot ID to match the intersection ID
    sem_init(&lot->spots, 0, PARKING_SPOTS); // Initialize the semaphore with 10 parking spots
    sem_init(&lot->queue, 0, PARKING_QUEUE); // Initialize the semaphore with 5 waiting spots
}

void destroy_parking(ParkingLot *lot) {
    if (lot == NULL) {
        safe_print("Error no parking lot provided\n");
        return;
    }
    sem_destroy(&lot->spots); // Destroy the semaphore for parking spots
    sem_destroy(&lot->queue); // Destroy the semaphore for waiting queue
}

ParkingLot *get_parking(int id) {
    if (id == 10) {
        return &park10; // Return pointer to parking lot for intersection 10
    } else if (id == 11) {
        return &park11; // Return pointer to parking lot for intersection 11
    } else {
        safe_print("Error invalid parking lot ID: %d\n", id);
        return NULL; // Invalid ID, return NULL
    }
}

int try_parking(Vehicle *v, ParkingLot *lot) {
    if (v == NULL || lot == NULL) {
        safe_print("Error no vehicle or parking lot provided\n");
        return -1;
    }
    if(sem_trywait(&lot->spots) == 0) {
        // Successfully acquired a parking spot
    }
    else{
        if(sem_trywait(&lot->queue) == 0){
            safe_print("[V%02d | %-9s] entered parking QUEUE at F%d\n",
               v->id, v->type, lot->id);

            sem_wait(&lot->spots); // Wait for a parking spot to become available
            sem_post(&lot->queue); // Leave the waiting queue
        }
        else{
            safe_print("[V%02d | %-9s] parking queue FULL at F%d — skipping\n",
                   v->id, v->type, lot->id);
            return 0;
        }
    }

     int park_sec = rand() % 5 + 2;  // 2 to 6 seconds
    safe_print("[V%02d | %-9s] PARKED    at F%d for %ds\n",
               v->id, v->type, lot->id, park_sec);
    sleep(park_sec);

    sem_post(&lot->spots); // Release the parking spot
    safe_print("[V%02d | %-9s] LEFT      parking at F%d\n",
               v->id, v->type, lot->id);
    return 1;
}


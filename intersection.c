#include "intersection.h"


Intersection* createIntersection(Intersection* intersection, int id) {
    if (intersection == NULL) {
        safe_print("Error no intersection provided\n");
        return NULL;
    }
    intersection->id = id; // Set the provided ID
    intersection->green = 1; // Initially, the light is green
    intersection->emergencyActive = 0; // No emergency at the start
    intersection->carsInside = 0; // No cars inside the intersection
    pthread_mutex_init(&intersection->mutex, NULL); // Initialize the mutex
    return intersection;
}

void destroyIntersection(Intersection* intersection) {
    if (intersection == NULL) {
        safe_print("Error no intersection provided\n");
        return;
    }
    pthread_mutex_destroy(&intersection->mutex); // Destroy the mutex
}

Intersection* getIntersection(int id) {
    if (id == 10) {
        return &f10; // Return pointer to intersection f10
    } else if (id == 11) {
        return &f11; // Return pointer to intersection f11
    } else {
        safe_print("Error invalid intersection ID: %d\n", id);
        return NULL; // Invalid ID, return NULL
    }
}

void crossIntersection(Intersection* intersection, Vehicle* vehicle) {

    safe_print("[V%02d | %-9s] approaching F%d  from %-5s -> %s\n",
               vehicle->id, vehicle->type, intersection->id, vehicle->origin, vehicle->destination);
    
    if (intersection == NULL || vehicle == NULL) {
        safe_print("Error no intersection or vehicle provided\n");
        return;
    }
    while (1) {
        pthread_mutex_lock(&intersection->mutex); // Lock the mutex to check the conditions
        if (intersection->green == 1 && intersection->emergencyActive == 0) {
            intersection->carsInside++; // Increment the count of cars inside the intersection
            safe_print("[V%02d | %-9s] CROSSING  F%d  [%d inside]\n",
                       vehicle->id, vehicle->type, intersection->id, intersection->carsInside);
            pthread_mutex_unlock(&intersection->mutex); // Unlock the mutex after updating the state
            break; // Exit the loop to cross the intersection
        }

        safe_print("[V%02d | %-9s] WAITING   F%d  (red/emergency)\n",
                   vehicle->id, vehicle->type, intersection->id);
        pthread_mutex_unlock(&intersection->mutex); // Unlock the mutex if conditions are not met
        usleep(1000); // Sleep for a short time before checking again to avoid busy waiting around 1ms
    }
    // Simulate the time taken to cross the intersection
    sleep(1); // Sleep for 1 second to simulate crossing time
    pthread_mutex_lock(&intersection->mutex); // Lock the mutex to update the state after crossing
    intersection->carsInside--; // Decrement the count of cars inside the intersection
    safe_print("[V%02d | %-9s] EXITED    F%d  [%d inside]\n",
                   vehicle->id, vehicle->type, intersection->id, intersection->carsInside);
    pthread_mutex_unlock(&intersection->mutex); // Unlock the mutex after updating the state
    sleep(1); // Sleep for 1 second to simulate time taken to clear the intersection after crossing
}
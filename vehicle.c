#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "vehicle.h"
#include "globals.h"
#include "helper.h"
#include "intersection.h"
#include "parking.h"
#include "controller.h"


Vehicle create_vehicle(int id) {
    Vehicle v;
    v.id = id;
    // Randomly assign a type
    int type_rand = rand() % 6;
    switch (type_rand) {
        case 0: strncpy(v.type, TYPE_AMBULANCE, sizeof(v.type)); break;
        case 1: strncpy(v.type, TYPE_FIRETRUCK, sizeof(v.type)); break;
        case 2: strncpy(v.type, TYPE_BUS, sizeof(v.type)); break;
        case 3: strncpy(v.type, TYPE_CAR, sizeof(v.type)); break;
        case 4: strncpy(v.type, TYPE_BIKE, sizeof(v.type)); break;
        case 5: strncpy(v.type, TYPE_TRACTOR, sizeof(v.type)); break;
    }
    // Randomly assign an origin
    int origin_rand = rand() % 4;
    switch (origin_rand) {
        case 0: strncpy(v.origin, ORIG_NORTH, sizeof(v.origin)); break;
        case 1: strncpy(v.origin, ORIG_SOUTH, sizeof(v.origin)); break;
        case 2: strncpy(v.origin, ORIG_EAST, sizeof(v.origin)); break;
        case 3: strncpy(v.origin, ORIG_WEST, sizeof(v.origin)); break;
    }
    // Randomly assign a destination
    int dest_rand = rand() % 3;
    switch (dest_rand) {
        case 0: strncpy(v.destination, DEST_STRAIGHT, sizeof(v.destination)); break;
        case 1: strncpy(v.destination, DEST_LEFT, sizeof(v.destination)); break;
        case 2: strncpy(v.destination, DEST_RIGHT, sizeof(v.destination)); break;
    }
    // Set priority based on type
    if (strcmp(v.type, TYPE_AMBULANCE) == 0 || strcmp(v.type, TYPE_FIRETRUCK) == 0) {
        v.priority = PRIO_EMERGENCY;
    } else if (strcmp(v.type, TYPE_BUS) == 0) {
        v.priority = PRIO_BUS;
    } else {
        v.priority = PRIO_NORMAL;
    }
    v.arrival_time = time(NULL); // Set arrival time to current time
    v.wants_to_park = rand() % 2; // Randomly decide if the vehicle wants to park
    v.target_intersection = (rand() % 2) ? 10 :11; // Randomly assign target intersection (10 or 11)
    return v;
}

void *vehicle_lifecycle(void *arg) {
    Vehicle *v = (Vehicle *)arg;
    if (v == NULL) {
        safe_print("Error no vehicle provided\n");
        return NULL;
    }
    
    Intersection *inter = getIntersection(v->target_intersection);
    ParkingLot *lot = get_parking(v->target_intersection);
    safe_print("[V%02d | %-9s] spawned   origin=%-5s dest=%-8s F%d  prio=%d\n",
               v->id, v->type, v->origin, v->destination,
               v->target_intersection, v->priority);
    if (inter == NULL) {
        safe_print("Error target intersection not found for vehicle %d\n", v->id);
        return NULL;
    }
    if (lot == NULL) {
        safe_print("Error target parking lot not found for vehicle %d\n", v->id);
        return NULL;
    }

    if(v->priority == PRIO_EMERGENCY) {
        sleep(1); // Simulate time taken to approach the intersection
        trigger_emergency(v, inter);
        safe_print("[V%02d | %-9s] DONE\n\n", v->id, v->type);
        sleep(1); // Simulate time taken to exit after crossing the intersection
        return NULL;
    } else {


        sleep(1); // Simulate time taken to approach the intersection

        if (v->wants_to_park) {
            int parked = try_parking(v, lot);

            if (parked) {
                safe_print("[V%02d | %-9s] DONE (parked — did not cross)\n\n",
                           v->id, v->type);
                return NULL;
            }

        // parking failed — fall through to normal crossing
        safe_print("[V%02d | %-9s] parking failed — crossing instead\n",
                   v->id, v->type);
        }

        crossIntersection(inter, v);
        sleep(1); // Simulate time taken to exit after crossing the intersection
        if(will_reach_other(v)) {
            // If the vehicle will reach the other intersection, we need to cross it as well
            Intersection *other_inter = getIntersection((v->target_intersection == 10) ? 11 : 10);
            if (other_inter == NULL) {
                safe_print("Error other intersection not found for vehicle %d\n", v->id);
                return NULL;
            }
            safe_print("[V%02d | %-9s] travelling F%d -> F%d\n",
                   v->id, v->type, inter->id, other_inter->id);
            crossIntersection(other_inter, v);
            sleep(1); // Simulate time taken to exit after crossing the second intersection
        }
        safe_print("[V%02d | %-9s] DONE\n\n", v->id, v->type);
        return NULL;

    }
}

int will_reach_other(Vehicle *v) {

    // West + Straight → heads East → reaches other intersection
    if (strcmp(v->origin, ORIG_WEST) == 0 &&
        strcmp(v->destination, DEST_STRAIGHT) == 0)
        return 1;

    // East + Straight → heads West → reaches other intersection
    if (strcmp(v->origin, ORIG_EAST) == 0 &&
        strcmp(v->destination, DEST_STRAIGHT) == 0)
        return 1;

    // North + Right → turns East → reaches other intersection
    // BUT only if target is F10 (turning right from North at F10 goes East toward F11)
    // At F11 turning right from North goes East → exits (no more intersections)
    if (strcmp(v->origin, ORIG_NORTH) == 0 &&
        strcmp(v->destination, DEST_RIGHT) == 0 &&
        v->target_intersection == 10)
        return 1;

    // South + Left → turns East → reaches other intersection
    if (strcmp(v->origin, ORIG_SOUTH) == 0 &&
        strcmp(v->destination, DEST_LEFT) == 0 &&
        v->target_intersection == 10)
        return 1;

    // North + Left → turns West → reaches other intersection
    // only if starting at F11
    if (strcmp(v->origin, ORIG_NORTH) == 0 &&
        strcmp(v->destination, DEST_LEFT) == 0 &&
        v->target_intersection == 11)
        return 1;

    // South + Right → turns West → reaches other intersection
    // only if starting at F11
    if (strcmp(v->origin, ORIG_SOUTH) == 0 &&
        strcmp(v->destination, DEST_RIGHT) == 0 &&
        v->target_intersection == 11)
        return 1;

    return 0;  // all other combinations exit after one intersection
}
#ifndef TYPES_H
#define TYPES_H

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define NUM_VEHICLES     15
#define PARKING_SPOTS    10
#define PARKING_QUEUE     5

// Priority levels (lower number = higher priority)
#define PRIO_EMERGENCY    1
#define PRIO_BUS          2
#define PRIO_NORMAL       3

// Pipe message tokens
#define MSG_EMERGENCY    "EMERGENCY_INCOMING"
#define MSG_CLEARED      "EMERGENCY_CLEARED"
#define MSG_LEN           32

// Vehicle types
#define TYPE_AMBULANCE   "Ambulance"
#define TYPE_FIRETRUCK   "Firetruck"
#define TYPE_BUS         "Bus"
#define TYPE_CAR         "Car"
#define TYPE_BIKE        "Bike"
#define TYPE_TRACTOR     "Tractor"

// Destinations
#define DEST_STRAIGHT    "Straight"
#define DEST_LEFT        "Left"
#define DEST_RIGHT       "Right"

// Origins
#define ORIG_NORTH       "North"
#define ORIG_SOUTH       "South"
#define ORIG_EAST        "East"
#define ORIG_WEST        "West"


typedef struct {
    int    id;                   // 0 to 14
    char   type[16];             // "Ambulance", "Car", etc.
    char   origin[8];            // "North", "South", "East", "West"
    char   destination[10];      // "Straight", "Left", "Right"
    int    priority;             // PRIO_EMERGENCY / PRIO_BUS / PRIO_NORMAL
    time_t arrival_time;         // set just before pthread_create()
    int    wants_to_park;        // 1 = try parking, 0 = skip
    int    target_intersection;  // 10 or 11
} Vehicle;

typedef struct {
    int id;                     // 10 or 11
    int green;                  // 1 = green, 0 = red
    int emergencyActive;        // 1 = emergency vehicle is crossing, 0 = no emergency
    int carsInside;             // number of vehicles currently crossing the intersection
    pthread_mutex_t mutex;      // protects access to the intersection's state
    pthread_cond_t cond;        // condition variable to notify waiting vehicles
} Intersection;

typedef struct {
    int id;                     // 10 or 11
    sem_t spots;              // counts available parking spots
    sem_t queue;              // limits number of vehicles waiting to park
} ParkingLot;

#endif
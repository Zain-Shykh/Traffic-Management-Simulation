#include "globals.h"



Intersection f10;
Intersection f11;
ParkingLot park10;
ParkingLot park11;
Vehicle   vehicles[NUM_VEHICLES];
pthread_t threads[NUM_VEHICLES];
int pipe_f10_to_f11[2];
int pipe_f11_to_f10[2];
pthread_mutex_t print_lock;   
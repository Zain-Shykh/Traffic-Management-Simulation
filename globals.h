#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include "structs.h"


extern Intersection f10;
extern Intersection f11;



extern ParkingLot park10;
extern ParkingLot park11;



extern Vehicle   vehicles[NUM_VEHICLES];
extern pthread_t threads[NUM_VEHICLES];


extern int pipe_f10_to_f11[2];
extern int pipe_f11_to_f10[2];


extern pthread_mutex_t print_lock;

#endif
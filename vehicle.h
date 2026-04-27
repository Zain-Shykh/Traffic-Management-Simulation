// vehicle.h
#ifndef VEHICLE_H
#define VEHICLE_H

#include "structs.h"

Vehicle create_vehicle(int id);


void *vehicle_lifecycle(void *arg);

int will_reach_other(Vehicle* v);

#endif // VEHICLE_H
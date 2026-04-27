#ifndef PARKING_H
#define PARKING_H

#include "structs.h"

void init_parking(ParkingLot *lot, int intersection_id);

void destroy_parking(ParkingLot *lot);

int  try_parking(Vehicle *v, ParkingLot *lot);

ParkingLot *get_parking(int id);

#endif // PARKING_H
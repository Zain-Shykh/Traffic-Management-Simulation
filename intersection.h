#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "structs.h"
#include "helper.h"

Intersection* createIntersection(Intersection* intersection, int id);

void destroyIntersection(Intersection* intersection);

void crossIntersection(Intersection* intersection, Vehicle* vehicle);

Intersection* getIntersection(int id);
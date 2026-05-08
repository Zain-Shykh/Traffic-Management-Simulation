#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "controller.h"
#include "globals.h"
#include "helper.h"
#include "intersection.h"

void run_controller(int id) {
    Intersection *inter = getIntersection(id);
    if (inter == NULL) {
        printf("Error: Intersection not found\n");
        return;
    }
    int read_fd;
    if (id == 10) {
        read_fd = pipe_f11_to_f10[0];
        close(pipe_f11_to_f10[1]); // Close unused write end
        close(pipe_f10_to_f11[0]); // Close unused read end
        close(pipe_f10_to_f11[1]); // Controller only reads; close writer to allow clean EOF shutdown
    } else if (id == 11) {
        read_fd = pipe_f10_to_f11[0];
        close(pipe_f10_to_f11[1]); // Close unused write end
        close(pipe_f11_to_f10[0]); // Close unused read end
        close(pipe_f11_to_f10[1]); // Controller only reads; close writer to allow clean EOF shutdown
    } else {
        printf("Error: Invalid controller ID\n");
        return;
    }
    printf("[Controller F%d] started — waiting on pipe...\n", id);
    fflush(stdout);
    while (1) {
        char buffer[MSG_LEN];
        ssize_t bytes_read = read(read_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            printf("[Controller F%d] pipe closed — exiting\n", id);
            fflush(stdout);
            break;
        }
        buffer[bytes_read] = '\0';
        printf("[Controller F%d] received: \"%s\"\n", id, buffer);
        fflush(stdout);
        if(strncmp(buffer, MSG_EMERGENCY, strlen(MSG_EMERGENCY)) == 0) {
            pthread_mutex_lock(&inter->mutex);
            inter->emergencyActive = 1;
            inter->green = 0;
            pthread_cond_broadcast(&inter->cond);
            pthread_mutex_unlock(&inter->mutex);
            printf("[Controller F%d] intersection CLEARED for emergency\n", id);
            fflush(stdout);
        }
        else if(strncmp(buffer, MSG_CLEARED, strlen(MSG_CLEARED)) == 0) {
            pthread_mutex_lock(&inter->mutex);
            inter->emergencyActive = 0;
            inter->green = 1;
            pthread_cond_broadcast(&inter->cond);
            pthread_mutex_unlock(&inter->mutex);
            printf("[Controller F%d] traffic RESTORED\n", id);
            fflush(stdout);
        }
        else {
            printf("Error: Unknown message received: %s\n", buffer);
        }
    }
    close(read_fd);
    exit(0);
}

void trigger_emergency(Vehicle *v, Intersection *inter) {
    if (v == NULL || inter == NULL) {
        safe_print("Error: Vehicle or Intersection is NULL\n");
        return;
    }
    safe_print("\n[EMERGENCY] %s V%02d at F%d — clearing path!\n",
               v->type, v->id, inter->id);

    int write_fd;
    if (inter->id == 10) {
        write_fd = pipe_f10_to_f11[1];
    } else if (inter->id == 11) {
        write_fd = pipe_f11_to_f10[1];
    } else {
        safe_print("Error: Invalid intersection ID\n");
        return;
    }

    // Set emergencyActive and green flags directly for own intersection
    pthread_mutex_lock(&inter->mutex);
    inter->emergencyActive = 1;
    inter->green = 0;
    pthread_cond_broadcast(&inter->cond);
    pthread_mutex_unlock(&inter->mutex);

    // Notify the other controller about the emergency
    safe_print("[V%02d | %-9s] TRIGGERING EMERGENCY at F%d!\n", v->id, v->type, inter->id);
    write(write_fd, MSG_EMERGENCY, strlen(MSG_EMERGENCY));
    usleep(300000); // Simulate time taken for emergency to clear the intersection

    // Cross the intersection exclusively for the emergency vehicle
    pthread_mutex_lock(&inter->mutex);
    inter->carsInside++;
    safe_print("[EMERGENCY] %s V%02d CROSSING F%d (exclusive)\n",
                   v->type, v->id, inter->id);
    pthread_mutex_unlock(&inter->mutex);


    sleep(1); // Simulate time taken to cross the intersection

    // Clear the intersection
    pthread_mutex_lock(&inter->mutex);
    inter->emergencyActive = 0;
    inter->green = 1;
    inter->carsInside--;
    pthread_cond_broadcast(&inter->cond);
    pthread_mutex_unlock(&inter->mutex);

    sleep(2);//simulate the time taken to cross the other intersection after the this one

    // Notify the other controller that the emergency has cleared
    safe_print("[EMERGENCY] %s V%02d cleared F%d — traffic restored\n\n",
               v->type, v->id, inter->id);
    write(write_fd, MSG_CLEARED, strlen(MSG_CLEARED));
}
#define _GNU_SOURCE
// main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include "globals.h"
#include "helper.h"
#include "intersection.h"
#include "parking.h"
#include "vehicle.h"
#include "controller.h"

static volatile sig_atomic_t stop_requested = 0;
static pid_t g_pid_f10_ctrl = -1;
static pid_t g_pid_f11_ctrl = -1;

void handle_sigint(int sig) {
    (void)sig;
    stop_requested = 1;
    write(STDOUT_FILENO, "\n[SIGINT] shutting down...\n", 27);
}


void shutdown(void) {
    destroy_parking(&park10);
    destroy_parking(&park11);
    destroyIntersection(&f10);
    destroyIntersection(&f11);
    pthread_mutex_destroy(&print_lock);
    close(pipe_f10_to_f11[0]);
    close(pipe_f10_to_f11[1]);
    close(pipe_f11_to_f10[0]);
    close(pipe_f11_to_f10[1]);
    printf("[shutdown] all resources freed\n");
}


int main(void) {
    int threads_started = 0;
    int joined[NUM_VEHICLES] = {0};

    srand((unsigned)time(NULL));

    printf("╔══════════════════════════════════════════╗\n");
    printf("║    F10 / F11 Traffic Simulation           ║\n");
    printf("║    Press Ctrl+C to stop                   ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    // ── Step 1: register SIGINT handler ──
    signal(SIGINT, handle_sigint);

    // ── Step 2: init print mutex ──
    pthread_mutex_init(&print_lock, NULL);

    // ── Step 3: init intersections ──
    createIntersection(&f10, 10);
    createIntersection(&f11, 11);
    printf("[init] F10 ready\n");
    printf("[init] F11 ready\n");

    // ── Step 4: init parking lots ──
    init_parking(&park10, 10);
    init_parking(&park11, 11);
    printf("[init] P10 ready  spots=%d  queue=%d\n", PARKING_SPOTS, PARKING_QUEUE);
    printf("[init] P11 ready  spots=%d  queue=%d\n\n", PARKING_SPOTS, PARKING_QUEUE);

    // ── Step 5: create pipes BEFORE fork ──
    if (pipe(pipe_f10_to_f11) < 0) {
        perror("pipe f10_to_f11");
        return 1;
    }
    if (pipe(pipe_f11_to_f10) < 0) {
        perror("pipe f11_to_f10");
        return 1;
    }
    printf("[init] pipes created\n\n");

    // ── Step 6: fork F10 controller ──
    pid_t pid_f10_ctrl = fork();
    if (pid_f10_ctrl < 0) {
        perror("fork F10");
        return 1;
    }
    if (pid_f10_ctrl == 0) {
        signal(SIGINT, SIG_IGN);
        // inside child process — never returns
        run_controller(10);
    }
    g_pid_f10_ctrl = pid_f10_ctrl;

    // ── Step 7: fork F11 controller ──
    pid_t pid_f11_ctrl = fork();
    if (pid_f11_ctrl < 0) {
        perror("fork F11");
        return 1;
    }
    if (pid_f11_ctrl == 0) {
        signal(SIGINT, SIG_IGN);
        // inside child process — never returns
        run_controller(11);
    }
    g_pid_f11_ctrl = pid_f11_ctrl;

    // ── back in parent only from here ──
    printf("[init] F10 controller  PID=%d\n", pid_f10_ctrl);
    printf("[init] F11 controller  PID=%d\n\n", pid_f11_ctrl);

    // ── Step 8: spawn 15 vehicle threads ──
    printf("[main] spawning %d vehicles...\n\n", NUM_VEHICLES);

    for (int i = 0; i < NUM_VEHICLES; i++) {
        if (stop_requested) {
            break;
        }
        vehicles[i] = create_vehicle(i);
        pthread_create(&threads[i], NULL, vehicle_lifecycle, &vehicles[i]);
        threads_started++;

        // random delay between spawns — 200ms to 600ms
        usleep((rand() % 400 + 200) * 1000);
    }

    // ── Step 9: wait for started threads (interruptible by SIGINT) ──
    int remaining = threads_started;
    while (remaining > 0) {
        if (stop_requested) {
            for (int i = 0; i < threads_started; i++) {
                if (!joined[i]) {
                    pthread_cancel(threads[i]);
                }
            }
        }

        for (int i = 0; i < threads_started; i++) {
            if (joined[i]) {
                continue;
            }

            int rc = pthread_tryjoin_np(threads[i], NULL);
            if (rc == 0) {
                joined[i] = 1;
                remaining--;
            } else if (rc != EBUSY) {
                joined[i] = 1;
                remaining--;
            }
        }

        if (remaining > 0) {
            usleep(50000);
        }
    }
    printf("[main] all vehicles finished\n\n");

    // ── Step 10: shut down controller processes ──
    close(pipe_f10_to_f11[1]);
    close(pipe_f11_to_f10[1]);

    if (stop_requested) {
        if (g_pid_f10_ctrl > 0) {
            kill(g_pid_f10_ctrl, SIGTERM);
        }
        if (g_pid_f11_ctrl > 0) {
            kill(g_pid_f11_ctrl, SIGTERM);
        }
    }

    // wait for both children to exit
    waitpid(pid_f10_ctrl, NULL, 0);
    waitpid(pid_f11_ctrl, NULL, 0);
    printf("[main] controllers exited\n\n");

    // ── Step 11: free all resources ──
    shutdown();

    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║         Simulation complete               ║\n");
    printf("╚══════════════════════════════════════════╝\n");

    return 0;
}
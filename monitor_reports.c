#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define PID_FILE ".monitor_pid"

/* Functie de tratare a semnalului (signal handler)*/
void handle_sigint(int signum) {
    char msg[128];
    snprintf(msg, sizeof(msg), "\n[MONITOR] Am captat semnalul %d (SIGINT). Inchidere.\n", signum);
    write(1, msg, strlen(msg)); 
    unlink(PID_FILE);
    exit(0);
}

/* Functie de tratare pentru semnalul definit de utilizator */
void handle_sigusr1(int signum) {
    char msg[128];
    snprintf(msg, sizeof(msg), "[MONITOR] Semnalul %d (SIGUSR1) captat: Raport nou!\n", signum);
    write(1, msg, strlen(msg)); 
}

int main(void) {
    // 1. FAZA 3: Verificare dacă rulează deja un monitor 
    FILE *f_check = fopen(PID_FILE, "r");
    if (f_check) {
        int existing_pid;
        if (fscanf(f_check, "%d", &existing_pid) == 1) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "ERR_PID:%d\n", existing_pid);
            write(1, error_msg, strlen(error_msg));
            fclose(f_check);
            exit(EXIT_FAILURE); 
        }
        fclose(f_check);
    }
    
    FILE *f = fopen(PID_FILE, "w");
    if (!f) {
        perror("[EROARE] Nu s-a putut crea fisierul .monitor_pid");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "%d", getpid());
    fclose(f);

    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));

    act.sa_handler = handle_sigint;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0; 
    sigaction(SIGINT, &act, NULL);

    act.sa_handler = handle_sigusr1;
    sigaction(SIGUSR1, &act, NULL);

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "[MONITOR] Pornit (PID: %d). Astept semnale...\n", getpid());
    write(1, start_msg, strlen(start_msg));

    /* Suspendarea executiei procesului folosind sleep() */
    while (1) {
        sleep(10); //
    }

    return 0;
}
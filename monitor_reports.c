#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define PID_FILE ".monitor_pid"

void handle_sigint(int sig) {
    (void)sig;
    const char msg[] = "\n[MONITOR] Semnal SIGINT primit. Inchidere monitor\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    unlink(PID_FILE);

    _exit(0);
}

void handle_sigusr1(int sig) {
    (void)sig;
    const char msg[] = "[MONITOR] Notificare primita: Un raport nou a fost adaugat!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main(void) {
    FILE *f = fopen(PID_FILE, "w");
    if (!f) {
        perror("[EROARE] Nu s-a putut crea fisierul .monitor_pid");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "%d", getpid());
    fclose(f);


    struct sigaction sa;
    memset(&sa, 0x00, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;


    sa.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("Eroare sigaction SIGINT");
        unlink(PID_FILE);
        exit(-1);
    }

    sa.sa_handler = handle_sigusr1;
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("Eroare sigaction SIGUSR1");
        unlink(PID_FILE);
        exit(-1);
    }

    printf("[MONITOR] Pornit (PID: %d). Astept semnale...\n", getpid());

    while (1) {
        pause();
    }

    return 0;
}

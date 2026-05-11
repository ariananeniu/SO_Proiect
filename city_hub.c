#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>


void start_monitor(){
    pid_t hub_mon;

    if((hub_mon=fork()) < 0){
        printf("Eroare la fork 1\n");
        exit(1);
    }

    if(hub_mon == 0){
        int pfd[2];
        pid_t monitor_pid;
        FILE *stream;


        if(pipe(pfd) < 0){
        printf("Eroare la crearea pipe-ululi\n");
        exit(1);
        }

        if((monitor_pid = fork()) < 0){
            printf("Eroare la fork 2\n");
            exit(1);
        }
        if(monitor_pid == 0){
            close(pfd[0]);
            dup2(pfd[1], 1);
            close(pfd[1]);

            execlp("./monitorreports","monitor_reports",NULL);

            printf("Eroare: nu s-a putut rula ./monitorreports\n");
            exit(1);
        }
        close(pfd[1]);

        stream = fdopen(pfd[0],"r");
        char buffer[512];

        while(fgets(buffer,sizeof(buffer),stream)){
            printf("-HUB MONITOR- a primit %s\n", buffer);
        }

        printf("-HUB MONITOR- Proces monitor oprit\n");
        fclose(stream);
        exit(0);
    }
    printf("-CITY_HUB- S-a pornit monitorul in fundal (PID: %d)\n", hub_mon);
}

int main(){
    printf("Test comanda start_monitor\n");
    start_monitor();

    while (1) {
        sleep(10); 
    }
    return 0;
}
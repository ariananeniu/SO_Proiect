#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

typedef struct {
    int id;
    char inspector[50];
    float lat, lon;
    char category[32];
    int severity;
    time_t timestamp;
    char description[256];
}Report;

typedef struct {
    char name[50];
    int score;
} InspectorScore;


int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Eroare: scorer nu a primit districtul\n");
        exit(1);
    }

    char *district = argv[1];
    char path[1024];

    snprintf(path, sizeof(path), "%s/reports.dat",district);

    FILE *file = fopen(path, "rb");
    if(file == NULL){
        printf("Niciun raport gasit\n");
        exit(1);
    }

    Report r;
    InspectorScore scores[100]; //maxim de 100 de inspectori diferiți
    int num_inspectors = 0;

    // Citim fișierul bucată cu bucată (pachete de mărimea structurii Report)
    while (fread(&r, sizeof(Report), 1, file) == 1) {
        int found = 0;
        
        // Căutăm dacă inspectorul citit acum este deja în lista noastră
        for (int i = 0; i < num_inspectors; i++) {
            if (strcmp(scores[i].name, r.inspector) == 0) {
                scores[i].score += r.severity; // Găsit -> Îi adunăm severitatea
                found = 1;
                break;
            }
        }

        
        if (!found && num_inspectors < 100) {
            strcpy(scores[num_inspectors].name, r.inspector);
            scores[num_inspectors].score = r.severity; // Setăm scorul inițial
            num_inspectors++;
        }
    }

    fclose(file);

    
    for (int i = 0; i < num_inspectors; i++) {
        printf("[%s] Inspector: %s | Workload Score: %d\n", district, scores[i].name, scores[i].score);
    }

    return 0;
}
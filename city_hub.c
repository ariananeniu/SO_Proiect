#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>


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

            execlp("./monitor_reports", "monitor_reports", NULL);

            printf("Eroare: nu s-a putut rula ./monitorreports\n");
            exit(1);
        }
        close(pfd[1]);

        stream = fdopen(pfd[0],"r");
        char buffer[512];

        while(fgets(buffer,sizeof(buffer),stream)){
            if (strncmp(buffer, "ERR_PID:", 8) == 0) {
                int existing_pid;
                sscanf(buffer, "ERR_PID:%d", &existing_pid);
                printf("-HUB MONITOR- Eroare: Un alt monitor rulează deja (PID: %d)\n", existing_pid);
                break; 
            } 
            else {
                printf("-HUB MONITOR- a primit %s", buffer);
            }
        }

        printf("-HUB MONITOR- Proces monitor oprit\n");
        fclose(stream);
        exit(0);
    }
    printf("-CITY_HUB- S-a pornit monitorul in fundal (PID: %d)\n", hub_mon);
}

//no_district - numarul de districte
//argv_district - tabloul care contine numele fiecarui district
void calculate_score(int no_distr, char *argv_district[]){
    int i;
    int array_pfd[no_distr];
    for(i=0; i<no_distr; i++){
        char *current_district = argv_district[i];

        int pfd[2];
        int pid;

        if(pipe(pfd)<0){ // creaza pipe-ul kernel
		printf("Eroare la crearea pipe-ului\n");
		exit(1);
	    }

        if((pid=fork()) < 0){
            printf("Eroare la fork()");
            exit(1);
        }

        if(pid == 0){
            close(pfd[0]); //copilul nu citeste din pipe, se inchide
            dup2(pfd[1],1); //face ca stdout sa pointeze spre pfd[1]
            close(pfd[1]); //pfd[1] nu mai e necesar
            execlp("./scorer","scorer",current_district,NULL);

            perror("Eroare: nu s-a putut rula scorer");
            exit(1);
        }
        close(pfd[1]);
        array_pfd[i] = pfd[0];  
    }
    FILE *stream;
    char buffer[512];
    for(i=0; i<no_distr; i++){
        stream = fdopen(array_pfd[i],"r");
        
        if(stream == NULL){
            perror("Eroare la fdopen\n");
            exit(1);
        }

        while(fgets(buffer, sizeof(buffer),stream) != NULL){
            printf("%s",buffer);
        }

        fclose(stream);
    }
    for(i = 0; i < no_distr; i++){
        wait(NULL);
    }
    
}

int main(){

    char input[1024];
    char *tokens[100];
    int token_count;

    printf("=== CITY INFRASTRUCTURE HUB (Faza 3) ===\n");

   
    while (1) {
        printf("city_hub> "); 
        if (fgets(input, sizeof(input), stdin) == NULL) break;

        //Eliminarea caracterului newline (\n) de la sfârșitul inputului
        input[strcspn(input, "\n")] = '\0';

        //Spargerea liniei în cuvinte separate prin spațiu 
        token_count = 0;
        char *space_pointer = strtok(input, " ");
        while (space_pointer != NULL && token_count < 100) {
            tokens[token_count++] = space_pointer;
            space_pointer = strtok(NULL, " ");
        }

        if (token_count == 0) continue; // Dacă utilizatorul a apăsat doar Enter, se trece mai departe

        //Identificarea și rutarea comenzilor 
        if (strcmp(tokens[0], "start_monitor") == 0) {
            start_monitor();
        } 
        else if (strcmp(tokens[0], "calculate_scores") == 0) {
            if (token_count < 2) {
                printf("Eroare: comanda cere o listă de districte. Format: calculate_scores <distr1> <distr2>...\n");
            } else {
                calculate_score(token_count - 1, tokens + 1);
            }
        } 
        else if (strcmp(tokens[0], "exit") == 0) {
            printf("Închidere City Hub.\n");
            break;
        } 
        else {
            printf("Comandă necunoscută! Comenzi valide: start_monitor, calculate_scores, exit\n");
        }
    }
    return 0;
}
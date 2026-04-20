#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
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

void get_permissions(mode_t mode, char *str) {
    strcpy(str, "---------");
    if (mode & S_IRUSR)
        str[0] = 'r';
    if (mode & S_IWUSR)
        str[1] = 'w';
    if (mode & S_IXUSR)
        str[2] = 'x';
    if (mode & S_IRGRP)
        str[3] = 'r';
    if (mode & S_IWGRP)
        str[4] = 'w';
    if (mode & S_IXGRP)
        str[5] = 'x';
    if (mode & S_IROTH)
        str[6] = 'r';
    if (mode & S_IWOTH)
        str[7] = 'w';
    if (mode & S_IXOTH)
        str[8] = 'x';
}

void list_reports(const char *district, const char *role) {
    char path[1024];
    sprintf(path, "%s/reports.dat", district);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Nu s-a gasit raportul in districtul '%s'.\n", district);
        return;
    }

    Report r;
    int count = 0;
    printf("Rapoarte:\n");

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        count++;
        printf("Raport %d:\n", count);
        printf("ID: %d\n", r.id);
        printf("Inspector: %s\n", r.inspector);
        printf("Categorie: %s\n", r.category);
        printf("Severitate: %d\n", r.severity);
        printf("Coordonate: [%.4f, %.4f]]\n",r.lat, r.lon);
        printf("Descriere: %s\n", r.description);

        printf("Data: %s", ctime(&r.timestamp));
        printf("----------------------------\n");
    }

    if (count == 0) {
        printf("Fisierul reports.dat exista, dar nu contine inregistrari\n");
    }

    close(fd);
}

void add(const char *district, const char *role, const char *user) {
    char path_dir[1024], path_file[1024], path_log[1024];

    Report r;
    memset(&r, 0, sizeof(Report));
    sprintf(path_dir, "%s", district);
    sprintf(path_file, "%s/reports.dat", district);
    sprintf(path_log, "%s/logged_district", district);

    mkdir(path_dir, 0750);
    chmod(path_dir, 0750);


    r.id = (int)time(NULL) % 10000;
    r.timestamp = time(NULL);
    strncpy(r.inspector, user, 50);

    printf("----- Introducere raport nou -----\n");
    printf("Categorie:\n");
    scanf("%31s", r.category);
    while (getchar() != '\n');

    printf("Severitate (1-minor, 2-moderat, 3-critic): ");
    scanf("%d", &r.severity);
    while (getchar() != '\n');

    printf("Coordonate (Latitudine longitudine): ");
    scanf("%f %f",&r.lat, &r.lon);
    while (getchar() != '\n');

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    printf("Descriere scurta problema: ");
    fgets(r.description, 255, stdin);
    r.description[strcspn(r.description, "\n")] = 0;

    int fd = open(path_file, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (fd == -1) {
        perror("Eroare la accesarea reports.dat");
        return;
    }

    chmod(path_file, 0664);

    if (write(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("Raport adaugat cu succes de catre %s (ID: %d)\n",user, r.id);
    }
    close(fd);

    int log_fd = open(path_log, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (log_fd != -1) {
        chmod(path_log, 0664);
        char log_entry[512];
        sprintf(log_entry, "[%ld] Role: %s, User %s, Action: ADD REPORT ID %d\n",
            time(NULL), role, user, r.id);
        write(log_fd, log_entry, strlen(log_entry));
        close(log_fd);
    }
}

int main(int argc, char *argv[]) {
    char *role = NULL;
    char *user = NULL;
    char *command = NULL;
    char *district = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        }
        else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        }
        else {
            if (command == NULL) {
                command = argv[i];
            } else if (district == NULL) {
                district = argv[i];
            }
        }
    }
    if (role == NULL || command == NULL || district == NULL) {
        printf("Utilizare corecta:\n");
        printf("%s --role <manager|inspector> [--user <nume>] <comanda> <district>\n", argv[0]);
        return 1;
    }

    if (strcmp(role, "manager") != 0 && strcmp(command, "inspector") != 0) {
        printf("Eroare: Rol invalid! (manager/inspector).\n");
        return 1;
    }


    if (strcmp(command, "add") == 0) {
        if (user == NULL) {
            printf("Comanda add necesita argumente --user <nume>.\n");
            return 1;
        }
        add(district, role, user);
    }
    else if (strcmp(command, "list") == 0) {
        printf("Lista rapoarte pentru %s (Rol: %s)\n", district, role);
        list_reports(district, role);
    }
    else {
        printf("Comanda necunoscuta.\n");
    }
    return 0;
}
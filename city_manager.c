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
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';
}

void check_access(const char *path, mode_t required_bit, const char *msg) {
    struct stat st;
    if (stat(path, &st) == -1) return; // Fișierul nu există încă, trecem mai departe
    if (!(st.st_mode & required_bit)) {
        fprintf(stderr, "[EROARE ACCES]: %s\n", msg);
        exit(EXIT_FAILURE);
    }
}

void log_action(const char *district, const char *role, const char *user, const char *action) {
    char path_log[1024];
    sprintf(path_log, "%s/logged_district", district);

    struct stat st;
    if (stat(path_log, &st) == 0) {
        if (strcmp(role, "inspector") == 0 && !(st.st_mode & S_IWGRP)) {
            printf("[AVERTISMENT] Inspectorii nu au drept de scriere in log.\n");
            return;
        }
    } else {
        if (strcmp(role, "inspector") == 0) {
            printf("[AVERTISMENT] Inspectorii nu au permisiunea de a crea/scrie in log.\n");
            return;
        }
    }


    int log_fd = open(path_log, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd != -1) {
        chmod(path_log, 0644);
        char log_entry[512];
        sprintf(log_entry, "[%ld] Role: %s, User: %s, Action: %s\n", time(NULL), role, user, action);
        write(log_fd, log_entry, strlen(log_entry));
        close(log_fd);
    }
}
void handle_symlink(const char *district) {
    char link_name[512], target_path[1024];
    sprintf(link_name, "active_reports-%s", district);
    sprintf(target_path, "%s/reports.dat", district);

    struct stat st;
    if (lstat(link_name, &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            struct stat st_target;
            if (stat(link_name, &st_target) == -1) {
                printf("[AVERTISMENT]: Link dangling detectat: %s\n", link_name);
            }
            unlink(link_name);
        }
    }
    symlink(target_path, link_name);
}

// --- Funcții generate cu asistența AI pentru Filter ---

int parse_condition(const char *input, char *field, char *op, char *value) {
    return sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    double r_val = 0, c_val = 0;
    char *r_str = NULL;

    if (strcmp(field, "severity") == 0) {
        r_val = r->severity; c_val = atof(value);
    } else if (strcmp(field, "timestamp") == 0) {
        r_val = (double)r->timestamp; c_val = atof(value);
    } else if (strcmp(field, "category") == 0) {
        r_str = r->category;
    } else if (strcmp(field, "inspector") == 0) {
        r_str = r->inspector;
    } else return 0;

    if (r_str) {
        if (strcmp(op, "==") == 0) return strcmp(r_str, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r_str, value) != 0;
        return 0;
    }

    if (strcmp(op, "==") == 0) return r_val == c_val;
    if (strcmp(op, "!=") == 0) return r_val != c_val;
    if (strcmp(op, "<") == 0)  return r_val < c_val;
    if (strcmp(op, "<=") == 0) return r_val <= c_val;
    if (strcmp(op, ">") == 0)  return r_val > c_val;
    if (strcmp(op, ">=") == 0) return r_val >= c_val;
    return 0;
}


void list_reports(const char *district, const char *role) {
    char path[1024];
    sprintf(path, "%s/reports.dat", district);

    // 1. OBTINEREA SI AFISAREA METADATELOR (Cerință obligatorie)
    struct stat st;
    if (stat(path, &st) == -1) {
        printf("Nu s-a gasit raportul in districtul '%s'.\n", district);
        return;
    }

    char perms[10];
    get_permissions(st.st_mode, perms);
    printf("--- METADATE FISIER ---\n");
    printf("Cale: %s\nPermisiuni: %s\nDimensiune: %lld bytes\nModificat: %s", path, perms, st.st_size, ctime(&st.st_mtime));
    printf("-----------------------\n");

    // 2. CITIREA SI AFISAREA CONTINUTULUI
    int fd = open(path, O_RDONLY);
    if (fd == -1) return;

    Report r;
    int count = 0;
    printf("Rapoarte:\n");

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        count++;
        printf("Raport %d:\nID: %d\nInspector: %s\nCategorie: %s\nSeveritate: %d\nCoordonate: [%.4f, %.4f]\nDescriere: %s\nData: %s",
               count, r.id, r.inspector, r.category, r.severity, r.lat, r.lon, r.description, ctime(&r.timestamp));
        printf("----------------------------\n");
    }

    if (count == 0) {
        printf("Fisierul reports.dat exista, dar nu contine inregistrari\n");
    }

    close(fd);
}

void add(const char *district, const char *role, const char *user) {
    char path_dir[1024], path_file[1024], path_cfg[1024];

    Report r;
    memset(&r, 0, sizeof(Report));
    sprintf(path_dir, "%s", district);
    sprintf(path_file, "%s/reports.dat", district);
    sprintf(path_cfg, "%s/district.cfg", district);

    mkdir(path_dir, 0750);
    chmod(path_dir, 0750);


    if (strcmp(role, "manager") == 0) check_access(path_file, S_IWUSR, "Managerul nu are drept de scriere!");
    else check_access(path_file, S_IWGRP, "Inspectorul nu are drept de scriere!");


    int cfg_fd = open(path_cfg, O_CREAT | O_EXCL | O_WRONLY, 0640);
    if (cfg_fd != -1) close(cfg_fd);

    r.id = (int)time(NULL) % 10000;
    r.timestamp = time(NULL);
    strncpy(r.inspector, user, 50);

    printf("----- Introducere raport nou -----\n");
    printf("Categorie:\n"); scanf("%31s", r.category);
    while (getchar() != '\n');
    printf("Severitate (1-minor, 2-moderat, 3-critic): "); scanf("%d", &r.severity);
    while (getchar() != '\n');
    printf("Coordonate (Latitudine longitudine): "); scanf("%f %f",&r.lat, &r.lon);
    while (getchar() != '\n');
    printf("Descriere scurta problema: "); fgets(r.description, 255, stdin);
    r.description[strcspn(r.description, "\n")] = 0;

    int fd = open(path_file, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (fd == -1) { perror("Eroare la accesarea reports.dat"); return; }
    chmod(path_file, 0664);

    if (write(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("Raport adaugat cu succes de catre %s (ID: %d)\n", user, r.id);


        char action[256];
        sprintf(action, "ADD REPORT ID %d", r.id);
        log_action(district, role, user, action);
    }
    close(fd);
}

void view(const char *district, int id_report) {
    char path_file[1024];
    sprintf(path_file, "%s/reports.dat", district);

    int fd = open(path_file, O_RDONLY);
    if (fd == -1) {
        perror("Eroare la accesarea reports.dat");
        return;
    }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == id_report) {
            found = 1;
            printf("Inspector: %s\n", r.inspector);
            printf("Categorie: %s\n", r.category);
            printf("Severitate: %d\n", r.severity);
            printf("Coordonate:  Lat: %.4f | Lon: %.4f\n", r.lat, r.lon);
            printf("Data:        %s", ctime(&r.timestamp));
            printf("------------------------------------------\n");
            printf("DESCRIERE:\n%s\n", r.description);
            printf("==========================================\n\n");
            break;
        }
    }
    if (!found) {
        printf("[!] Eroare: Raportul cu ID-ul %d nu a fost gasit in districtul %s.\n", id_report, district);
    }

    close(fd);
}

void remove_report(const char *district, int id) {
    char path[1024];
    sprintf(path, "%s/reports.dat", district);

    int fd = open(path, O_RDWR);
    if (fd == -1) { perror("Nu s-a putut deschide fisierul pentru stergere"); return; }

    Report r;
    int found = 0;
    off_t current_pos;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == id) {
            found = 1;
            current_pos = lseek(fd, 0, SEEK_CUR);
            Report next_r;

            // Shifting la stânga a restului fișierului
            while (read(fd, &next_r, sizeof(Report)) == sizeof(Report)) {
                lseek(fd, current_pos - sizeof(Report), SEEK_SET);
                write(fd, &next_r, sizeof(Report));
                lseek(fd, sizeof(Report), SEEK_CUR);
                current_pos = lseek(fd, 0, SEEK_CUR);
            }

            struct stat st;
            fstat(fd, &st);
            ftruncate(fd, st.st_size - sizeof(Report));
            printf("Raportul %d a fost sters.\n", id);
            break;
        }
    }
    if (!found) printf("Raportul %d nu a fost gasit.\n", id);
    close(fd);
}

void filter_reports(const char *district, int argc, char *argv[], int start_idx) {
    char path[1024];
    sprintf(path, "%s/reports.dat", district);
    int fd = open(path, O_RDONLY);
    if (fd == -1) { perror("Eroare la acces"); return; }

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int all_match = 1;
        for (int i = start_idx; i < argc; i++) {
            char field[32], op[5], value[64];
            if (parse_condition(argv[i], field, op, value)) {
                if (!match_condition(&r, field, op, value)) {
                    all_match = 0; break;
                }
            }
        }
        if (all_match) {
            printf("MATCH -> ID: %d | Cat: %s | Sev: %d | %s\n", r.id, r.category, r.severity, r.description);
        }
    }
    close(fd);
}

void update_threshold(const char *district, const char *value) {
    char path[1024];
    sprintf(path, "%s/district.cfg", district);

    struct stat st;
    if (stat(path, &st) == 0) {
        if ((st.st_mode & 0777) != 0640) {
            printf("[EROARE] Permisiunile fisierului district.cfg au fost modificate! Refuz operatiunea.\n");
            return;
        }
    }

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0640);
    if (fd != -1) {
        char buffer[256];
        sprintf(buffer, "threshold=%s\n", value);
        write(fd, buffer, strlen(buffer));
        close(fd);
        printf("Threshold actualizat la %s.\n", value);
    }
}


// Phase 2
void remove_district(const char *district, const char *role) {
    if (role == NULL || strcmp(role, "manager") != 0) {
        fprintf(stderr, "[EROARE] Acces refuzat: Doar managerul poate sterge un district.\n");
        return;
    }

    if (district == NULL || strlen(district) == 0 ||
        strcmp(district, ".") == 0 || strcmp(district, "..") == 0 || strchr(district, '/') != NULL) {
        fprintf(stderr, "[EROARE] Nume de district invalid sau periculos: '%s'.\n", district ? district : "NULL");
        return;
    }

    struct stat st;
    if (stat(district, &st) == -1) {
        perror("[EROARE] Districtul nu a putut fi gasit");
        return;
    }


    char link_name[512];
    snprintf(link_name, sizeof(link_name), "active_reports-%s", district);

    pid_t pid = fork();

    if (pid < 0) {
        perror("[EROARE] Fork a esuat");
        return;
    }

    if (pid == 0) {
        execlp("rm", "rm", "-rf", district, (char *)NULL);
        perror("[EROARE FIU] Executia 'rm' a esuat");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            if (unlink(link_name) == 0) {
                printf("[SUCCESS] Districtul '%s' si symlink-ul '%s' au fost eliminate.\n", district, link_name);
            } else
            {
                printf("[INFO] Districtul '%s' a fost sters, dar symlink-ul nu a putut fi eliminat (posibil inexistent).\n", district);
            }
        } else {
            fprintf(stderr, "[EROARE] Comanda 'rm' a esuat cu statusul %d.\n", WEXITSTATUS(status));
        }
    }
}
int main(int argc, char *argv[]) {
    char *role = NULL, *user = "unknown", *command = NULL, *district = NULL;
    int arg_idx = 1;


    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (command == NULL) {
            command = argv[i];
            arg_idx = i;
        } else if (district == NULL) {
            district = argv[i];
        }
    }

    if (role == NULL || command == NULL || district == NULL) {
        printf("Utilizare: %s --role <manager|inspector> [--user <nume>] <comanda> <district> [argumente]\n", argv[0]);
        return 1;
    }


    if (strcmp(role, "manager") != 0 && strcmp(role, "inspector") != 0) {
        printf("Eroare: Rol invalid! (manager/inspector).\n");
        return 1;
    }


    if (strcmp(command, "remove_district") == 0) {
        if (strcmp(role, "manager") != 0) {
            printf("Eroare: Doar managerul poate sterge un district.\n");
            return 1;
        }
        remove_district(district, role);
        return 0;
    }


    handle_symlink(district);


    if (strcmp(command, "add") == 0) {
        add(district, role, user);
    }

    else if (strcmp(command, "list") == 0) {
        list_reports(district, role);
    }

    else if (strcmp(command, "view") == 0) {
        if (arg_idx + 2 < argc) {
            view(district, atoi(argv[arg_idx + 2]));
        } else {
            int id_cautat;
            printf("Introduceti id-ul cautat: ");
            if (scanf("%d", &id_cautat) == 1) {
                view(district, id_cautat);
            }
        }
    }

    else if (strcmp(command, "remove_report") == 0) {
        if (strcmp(role, "manager") != 0) {
            printf("Eroare: Doar managerul poate sterge rapoarte.\n");
            return 1;
        }
        if (arg_idx + 2 < argc) {
            remove_report(district, atoi(argv[arg_idx + 2]));
        } else {
            printf("Argument lipsa: ID raport.\n");
        }
    }

    else if (strcmp(command, "update_threshold") == 0) {
        if (strcmp(role, "manager") != 0) {
            printf("Eroare: Doar managerul poate actualiza pragul.\n");
            return 1;
        }
        if (arg_idx + 2 < argc) {
            update_threshold(district, argv[arg_idx + 2]);
        } else {
            printf("Argument lipsa: valoare prag.\n");
        }
    }

    else if (strcmp(command, "filter") == 0) {
        filter_reports(district, argc, argv, arg_idx + 2);
    }

    else {
        printf("Comanda necunoscuta: %s\n", command);
        return 1;
    }

    return 0;
}
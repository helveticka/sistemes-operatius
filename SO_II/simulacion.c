#include "simulacion.h"

int acabados = 0;
char simul_dir[64];  // para /simul_aaaammddhhmmss

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo>\n", argv[0]);
        return FALLO;
    }

    signal(SIGCHLD, reaper); // Asociar la señal

    if (bmount(argv[1]) == -1) {
        perror("Error montando el dispositivo");
        return FALLO;
    }

    // Crear directorio simul_yyyymmddhhmmss
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    sprintf(simul_dir, "/simul_%04d%02d%02d%02d%02d%02d/",
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

    if (mi_creat(simul_dir, 6) < 0) {
        fprintf(stderr, "Error creando el directorio %s\n", simul_dir);
        bumount();
        return FALLO;
    }

    for (int i = 0; i < NUMPROCESOS; i++) {
        pid_t pid = fork();
        if (pid == 0) {  // Hijo
            if (bmount(argv[1]) == -1) {
                perror("Hijo: error montando dispositivo");
                exit(1);
            }

            pid_t mi_pid = getpid();
            char dir_proceso[100];
            sprintf(dir_proceso, "%sproceso_%d/", simul_dir, mi_pid);

            char camino[110];
            sprintf(camino, "%sprueba.dat", dir_proceso);
            if (mi_creat(dir_proceso, 6) < 0) {
                fprintf(stderr, "[%d] Error creando %s\n", mi_pid, dir_proceso);
                bumount();
                exit(1);
            }

            if (mi_creat(camino, 6) < 0) {
                fprintf(stderr, "[%d] Error creando %s\n", mi_pid, camino);
                bumount();
                exit(1);
            }

            srand(time(NULL) + mi_pid);
            struct REGISTRO reg;

            for (int j = 0; j < NUMESCRITURAS; j++) {
                reg.fecha = time(NULL);
                reg.pid = mi_pid;
                reg.nEscritura = j + 1;
                reg.nRegistro = rand() % REGMAX;

                off_t offset = reg.nRegistro * sizeof(struct REGISTRO);
                if (mi_write(camino, &reg, offset, sizeof(struct REGISTRO)) < 0) {
                    fprintf(stderr, "[%d] Error escribiendo en %s\n", mi_pid, camino);
                }

                my_sleep(50);
            }
#if DEBUGN12
            fprintf(stderr, GRAY"[Proceso %d: Completadas %d escrituras en %s]\n" RESET, i + 1, NUMESCRITURAS, camino);
#endif
            bumount();
            exit(0);
        }

        my_sleep(150);  // pausa entre procesos
    }

    while (acabados < NUMPROCESOS) {
        pause();
    }

    bumount();
    return 0;
}

void reaper(){
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
       acabados++;
    }
  }

void my_sleep(unsigned msec) { //recibe tiempo en milisegundos
    struct timespec req, rem;
    int err;
    req.tv_sec = msec / 1000; //conversión a segundos
    req.tv_nsec = (msec % 1000) * 1000000; //conversión a nanosegundos
    while ((req.tv_sec != 0) || (req.tv_nsec != 0)) {
        if (nanosleep(&req, &rem) == 0) 
        // rem almacena el tiempo restante si una llamada al sistema
        // ha sido interrumpida por una señal
            break;
        err = errno;
        // Interrupted; continue
        if (err == EINTR) {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
    }
}
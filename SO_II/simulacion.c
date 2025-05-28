#include "simulacion.h"

int acabados = 0;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo>\n", argv[0]);
        return -1;
    }

    // Montar sistema de ficheros
    if (bmount(argv[1]) == -1) {
        perror("Error montando el dispositivo");
        return -1;
    }

    // Crear directorio /simulacion/
    if (mi_creat("/simulacion", 6) < 0) {
        fprintf(stderr, "Error creando el directorio /simulacion\n");
        bumount();
        return -1;
    }

    signal(SIGCHLD, reaper);  // Activamos el reaper

    pid_t pid;
    for (int i = 0; i < NUMPROCESOS; i++) {
        pid = fork();
        if (pid == 0) {  // Hijo
            char camino[100];
            struct REGISTRO reg;

            pid_t mi_pid = getpid();
            sprintf(camino, "/simulacion/%d", mi_pid);

            if (mi_creat(camino, 6) < 0) {
                fprintf(stderr, "[%d] Error creando el fichero %s\n", mi_pid, camino);
                exit(1);
            }

            srand(time(NULL) ^ (getpid()<<16)); // Semilla diferente para cada hijo

            for (int j = 0; j < NUMESCRITURAS; j++) {
                reg.fecha = time(NULL);
                reg.pid = mi_pid;
                reg.nEscritura = j + 1;
                reg.nRegistro = rand() % REGMAX;

                if (mi_write(camino, &reg, reg.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO)) < 0) {
                    fprintf(stderr, "[%d] Error escribiendo en %s\n", mi_pid, camino);
                }
#if DEBUGN12
                fprintf(stderr, "[simulacion.c -> Escritura %d en %s]\n", reg.nEscritura, camino);
#endif
                my_sleep(rand() % 51);  // entre 0 y 50 ms
            }

            exit(0);  // Hijo termina
        }
    }

    // Esperamos a que terminen todos
    while (acabados < NUMPROCESOS) {
        pause();  // Espera activa hasta recibir SIGCHLD
    }

    printf("Todos los procesos han terminado (%d)\n", acabados);

    // Desmontar sistema
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
 
  
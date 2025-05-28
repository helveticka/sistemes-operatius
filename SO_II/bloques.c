/**
 * @file bloques.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "bloques.h"
#include "semaforo_mutex_posix.h"
// Variables globales
static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;
/**
 * @brief Monta el dispositivo virtual
 * @param camino Ruta del dispositivo virtual
 * @return Descriptor del dispositivo virtual, -1 si ha habido un error
 */
int bmount(const char *camino) {
    if (descriptor > 0) {
        close(descriptor); // cerramos el descriptor para que se pueda abrir con el bloque de tamaño correcto
    }
    if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem(); 
        if (mutex == SEM_FAILED) {
            return -1;
        }
    } 
    umask(000);
    
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    
    if (descriptor == FALLO) {
        fprintf(stderr, RED "Error al abrir el dispositivo virtual" RESET);
        return FALLO;
    }
    
    return descriptor;
}
/**
 * @brief Desmonta el dispositivo virtual
 * @return 0 si se ha desmontado correctamente, -1 si ha habido un error
 */
int bumount() {
    if (close(descriptor) == FALLO) {
        return FALLO;
    }
    deleteSem();
    return EXITO;
}
/**
 * @brief Escribe un bloque en el dispositivo virtual
 * @param nbloque Número de bloque
 * @param buf Buffer con los datos a escribir
 * @return Número de bytes escritos en el bloque, -1 si ha habido un error
 */
int bwrite(unsigned int nbloque, const void *buf) {
    lseek(descriptor, nbloque*BLOCKSIZE, SEEK_SET);
    if (write(descriptor, buf, BLOCKSIZE) == -1) {
        return FALLO;
    }
    return BLOCKSIZE;
}
/**
 * @brief Lee un bloque del dispositivo virtual
 * @param nbloque Número de bloque
 * @param buf Buffer donde se almacenarán los datos leídos
 * @return Número de bytes leídos, -1 si ha habido un error
 */
int bread(unsigned int nbloque, void *buf) {
    lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    size_t bytes_leidos = read(descriptor, buf, BLOCKSIZE);
    if (bytes_leidos == -1) {
        return FALLO;
    }
    return bytes_leidos;
}

void mi_waitSem() {
    if (!inside_sc) { // Si no estamos dentro de una sección crítica
        waitSem(mutex); // Entramos en la sección crítica
    }
    inside_sc++; // Incrementamos el contador de secciones críticas
}

void mi_signalSem() {
    inside_sc--; // Decrementamos el contador de secciones críticas
    if (!inside_sc) { // Si hemos salido de la última sección crítica
        signalSem(mutex); // Liberamos el semáforo
    }
}

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

#if MMAP
static void *ptrSFM = NULL;
#endif

static size_t tamSFM = 0;

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

#if MMAP
    ptrSFM = do_mmap(descriptor);
    if (ptrSFM == NULL) {
        return FALLO;
    }
#endif

    return descriptor;
}
/**
 * @brief Desmonta el dispositivo virtual
 * @return 0 si se ha desmontado correctamente, -1 si ha habido un error
 */
int bumount() {
#if MMAP
    if (msync(ptrSFM, tamSFM, MS_SYNC) == FALLO) {
        fprintf(stderr, RED "Error al sincronizar memoria: %s\n" RESET, strerror(errno));
        return FALLO;
    }
    if (munmap(ptrSFM, tamSFM) == FALLO) {
        fprintf(stderr, RED "Error al liberar memoria: %s\n" RESET, strerror(errno));
        return FALLO;
    }
#endif

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
#if MMAP
    size_t pos = nbloque * BLOCKSIZE;
    // Comprobamos si la posición está dentro del tamaño de la memoria
    if (pos >= tamSFM) {
        return FALLO;
    }
    size_t nbytes = BLOCKSIZE;
    // Comprobamos si la posición más el número de bytes a escribir supera el tamaño de la memoria
    if (pos + nbytes > tamSFM) {
        nbytes = tamSFM - pos; 
    }
    // Copiamos los datos al bloque
    memcpy(ptrSFM + pos, buf, nbytes);
    return nbytes;
#endif

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

#if MMAP
    size_t pos = nbloque * BLOCKSIZE;
    // Comprobamos si la posición está dentro del tamaño de la memoria
    if (pos >= tamSFM) {
        return FALLO;
    }
    size_t nbytes = BLOCKSIZE;
    // Comprobamos si la posición más el número de bytes a leer supera el tamaño de la memoria
    if (pos + nbytes > tamSFM) {
        nbytes = tamSFM - pos; 
    }
    // Copiamos los datos del bloque
    memcpy(buf, ptrSFM + pos, nbytes);
    return nbytes;
#endif

    lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    size_t bytes_leidos = read(descriptor, buf, BLOCKSIZE);
    if (bytes_leidos == -1) {
        return FALLO;
    }
    return bytes_leidos;
}
/**
 * @brief Espera a que el semáforo esté disponible para entrar en una sección crítica
 */
void mi_waitSem() {
    if (!inside_sc) { // Si no estamos dentro de una sección crítica
        waitSem(mutex); // Entramos en la sección crítica
    }
    inside_sc++; // Incrementamos el contador de secciones críticas
}
/**
 * @brief Libera el semáforo al salir de una sección crítica
 */
void mi_signalSem() {
    inside_sc--; // Decrementamos el contador de secciones críticas
    if (!inside_sc) { // Si hemos salido de la última sección crítica
        signalSem(mutex); // Liberamos el semáforo
    }
}

/**
 * @brief Mapea el dispositivo virtual a memoria compartida
 * @param fd Descriptor del dispositivo virtual
 */
void *do_mmap(int fd) {
    struct stat st;
    void *ptr;
    fstat(fd, &st);
    if (fstat(fd, &st) == FALLO) {
        fprintf(stderr, RED "Error fstat(): %s\n" RESET, strerror(errno));
        return NULL;
    }
    tamSFM = st.st_size; // tamaño memoria compartida
    ptr = mmap(NULL, tamSFM, PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, RED "Error %d: %s\n" RESET, errno, strerror(errno));
        return NULL;
    }
    return ptr;
}
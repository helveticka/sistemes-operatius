/**
 * @file bloques.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "bloques.h"
// Variables globales
static int descriptor = 0;
/**
 * @brief Monta el dispositivo virtual
 * @param camino Ruta del dispositivo virtual
 * @return Descriptor del dispositivo virtual, -1 si ha habido un error
 */
int bmount(const char *camino) {
    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == FALLO) {
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
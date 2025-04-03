/**
 * @file ficheros.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "ficheros.h"
/**
 * @brief Escribe el contenido procedente de un buffer de memoria en un fichero/directorio
 * @param ninodo Nº de inodo del fichero en el que se escribirá
 * @param buf_original Buffer de memoria con el contenido a escribir
 * @param offset Posición de escritura inicial, en bytes lógicos, con respecto al inodo
 * @param nbytes Nº de bytes a escribir
 * @return Cantidad de bytes escritos, FALLO en caso de error
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == -1) {
        fprintf(stderr, RED"Error al leer el inodo en mi_write_f\n" RESET);
        return FALLO;
    }
    // Verificar permisos de escritura
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, RED"No hay permisos de escritura\n" RESET);
        return FALLO;
    }
    // Cálculo de bloques y desplazamientos
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int nbfisico;
    unsigned int bytes_escritos = 0;
    // Caso donde todo cabe en un solo bloque lógico
    if (primerBL == ultimoBL) {
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (leer_inodo(ninodo, &inodo) == FALLO){
            fprintf(stderr, RED "Error al leer inodo en mi_write_f()"RESET);
            return FALLO;
        }
        if (nbfisico == FALLO) return FALLO;
        // Leer bloque del dispositivo
        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        // Copiar solo la parte necesaria desde buf_original a buf_bloque
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        // Escribir el bloque modificado
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        bytes_escritos += nbytes;
    } else {
        // 1. Primer bloque lógico (parcial)
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (nbfisico == FALLO) return FALLO;
        if (leer_inodo(ninodo, &inodo) == FALLO){
            fprintf(stderr, RED "Error al leer inodo en mi_write_f()"RESET);
            return FALLO;
        }
        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        unsigned int bytes_a_escribir = BLOCKSIZE - desp1;
        memcpy(buf_bloque + desp1, buf_original, bytes_a_escribir);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        bytes_escritos += bytes_a_escribir;
        // 2. Bloques intermedios (completos)
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 1);
            if (leer_inodo(ninodo, &inodo) == FALLO){
                fprintf(stderr, RED "Error al leer inodo en mi_write_f()"RESET);
                return FALLO;
            }
            if (nbfisico == FALLO) return FALLO;
            if (bwrite(nbfisico, buf_original + bytes_escritos) == -1) return -1;
            bytes_escritos += BLOCKSIZE;
        }
        // 3. Último bloque lógico (parcial)
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (leer_inodo(ninodo, &inodo) == FALLO){
            fprintf(stderr, RED "Error al leer inodo en mi_write_f()"RESET);
            return FALLO;
        }
        if (nbfisico == FALLO) return FALLO;
        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque, buf_original + bytes_escritos, desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        bytes_escritos += desp2 + 1;
    }
    // Actualizar inodo si es necesario
    if (offset + nbytes > inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog = offset + nbytes;
    }
    inodo.ctime = time(NULL);
    inodo.mtime = time(NULL);
    //inodo.numBloquesOcupados = (inodo.tamEnBytesLog + BLOCKSIZE - 1) / BLOCKSIZE;
    if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
    return bytes_escritos;
}
/**
 * @brief Lee el contenido de un fichero/directorio y lo almacena en un buffer de memoria
 * @param ninodo Nº de inodo del fichero a leer
 * @param buf_original Buffer de memoria donde se almacenará el contenido leído
 * @param offset Posición de lectura inicial, en bytes lógicos, con respecto al inodo
 * @param nbytes Nº de bytes a leer
 * @return Cantidad de bytes leídos, FALLO en caso de error
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    int leidos = 0;
    int nbfisico;
    char buf_bloque[BLOCKSIZE];
    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO){
        fprintf(stderr, RED "Error al leer inodo en mi_write_f()"RESET);
        return FALLO;
    }
    // Actualizar fecha de acceso
    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO){
        fprintf(stderr, RED "Error al escribir inodo en mi_write_f()"RESET);
        return FALLO;
    }
    // Verificar permisos de lectura
    if ((inodo.permisos & 4) != 4) {
       fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
       return FALLO;
    }
    // Verificar si el offset es mayor o igual que el tamaño lógico del inodo
    if (offset >= inodo.tamEnBytesLog) {
        leidos = 0;
        return leidos;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    // Caso donde todo cabe en un solo bloque lógico
    if (primerBL == ultimoBL) {
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) {
                fprintf(stderr, RED "Error al leer el bloque en mi_read_f()"RESET);
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        leidos += nbytes;
    } else {
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) {
                fprintf(stderr, RED "Error al leer el bloque en mi_read_f()"RESET);
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        leidos += BLOCKSIZE - desp1;
        for (int i = (primerBL + 1); i < ultimoBL; i++) {
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nbfisico != FALLO) {
                if (bread(nbfisico, buf_bloque) == FALLO) {
                    fprintf(stderr, RED "Error al leer el bloque en mi_read_f()"RESET);
                    return FALLO;
                }
                memcpy((buf_original+(BLOCKSIZE-desp1)+(i-primerBL-1)*BLOCKSIZE), buf_bloque, BLOCKSIZE);
            }
            leidos += BLOCKSIZE;
        }
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) {
                fprintf(stderr, RED "Error al leer el bloque en mi_read_f()"RESET);
                return FALLO;
            }
            memcpy((buf_original+(nbytes-desp2-1)), buf_bloque, desp2+1);
        }
        leidos += desp2 + 1;
    }
    if (leidos != nbytes) {
        fprintf(stderr, RED "Error al leer el inodo en mi_read_f()"RESET);
        return FALLO;
    }
    return leidos;
}
/**
 * @brief Obtiene los metadatos de un inodo
 * @param ninodo Número de inodo
 * @param p_stat Estructura donde se almacenarán los metadatos
 * @return EXITO si se han obtenido los metadatos correctamente, FALLO en caso contrario
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    struct inodo inodo;
    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == -1){
        return FALLO;
    }
    // Rellenar la estructura p_stat
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->btime = inodo.btime;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    return EXITO;
}
/**
 * @brief Cambia los permisos de un inodo
 * @param ninodo Número de inodo
 * @param permisos Permisos a establecer
 * @return EXITO si se han cambiado los permisos correctamente, FALLO en caso contrario
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    struct inodo inodo;
    // Leer el inodo
    if(leer_inodo(ninodo, &inodo) == -1){
        return FALLO;
    }
    // Modificar los permisos
    inodo.permisos = permisos;
    // Actualizar la fecha de modificación
    inodo.ctime = time(NULL);
    // Escribir el inodo
    if (escribir_inodo(ninodo, &inodo)==-1){
        return FALLO;
    }
    return EXITO;
}
/**
 * @brief Cambia la fecha de acceso de un inodo
 * @param ninodo Número de inodo
 * @param nbytes Número de bytes a leer
 * @return número de bloques liberados, FALLO en caso contrario
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    int primerBL, bloquesLiberados;
    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO){
        fprintf(stderr, RED "Error al leer el inodo en mi_truncar_f()"RESET);
        return FALLO;
    }
    // Verificar permisos de escritura
    if ((inodo.permisos & 2) != 2){
        fprintf(stderr, RED "No hay permisos de escritura"RESET);
        return FALLO;
    }
    // Verificar si nbytes es mayor que el tamaño lógico
    if (nbytes > inodo.tamEnBytesLog){
        fprintf(stderr, RED "Error debido a que nbytes mayor que el tamaño lógico en mi_truncar_f()"RESET);
        return FALLO;
    }
    if (nbytes % BLOCKSIZE == 0) {
        primerBL = nbytes / BLOCKSIZE;
    } else {
        primerBL = nbytes / BLOCKSIZE + 1;
    }
    bloquesLiberados = liberar_bloques_inodo(primerBL, &inodo);
    if (bloquesLiberados == FALLO){
        fprintf(stderr, RED "Error al liberar bloques en mi_truncar_f()"RESET);
        return FALLO;
    }
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= bloquesLiberados;
    if (escribir_inodo(ninodo, &inodo) == FALLO){
        fprintf(stderr, RED "Error al escribir inodo en mi_truncar_f()"RESET);
        return FALLO;
    }
    return bloquesLiberados;
}

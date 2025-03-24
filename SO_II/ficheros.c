#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
    
    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == -1) {
        fprintf(stderr, "Error al leer el inodo\n");
        return -1;
    }

    // Verificar permisos de escritura
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, "No hay permisos de escritura\n");
        return -1;
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
        if (nbfisico == -1) return -1;

        // Leer bloque del dispositivo
        if (bread(nbfisico, buf_bloque) == -1) return -1;

        // Copiar solo la parte necesaria desde buf_original a buf_bloque
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        // Escribir el bloque modificado
        if (bwrite(nbfisico, buf_bloque) == -1) return -1;

        bytes_escritos += nbytes;
    } else {
        // 1. Primer bloque lógico (parcial)
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (nbfisico == -1) return -1;

        if (bread(nbfisico, buf_bloque) == -1) return -1;

        unsigned int bytes_a_escribir = BLOCKSIZE - desp1;
        memcpy(buf_bloque + desp1, buf_original, bytes_a_escribir);
        if (bwrite(nbfisico, buf_bloque) == -1) return -1;

        bytes_escritos += bytes_a_escribir;

        // 2. Bloques intermedios (completos)
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 1);
            if (nbfisico == -1) return -1;

            if (bwrite(nbfisico, buf_original + bytes_escritos) == -1) return -1;

            bytes_escritos += BLOCKSIZE;
        }

        // 3. Último bloque lógico (parcial)
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (nbfisico == -1) return -1;

        if (bread(nbfisico, buf_bloque) == -1) return -1;

        memcpy(buf_bloque, buf_original + bytes_escritos, desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == -1) return -1;

        bytes_escritos += desp2 + 1;
    }

    // Actualizar inodo si es necesario
    if (offset + nbytes > inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog = offset + nbytes;
    }

    inodo.ctime = time(NULL);
    inodo.mtime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == -1) return -1;

    return bytes_escritos;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    unsigned int primerBL, ultimoBL;
    unsigned int bytesEscritos = 0;
    unsigned int bytesEscribiendo = 0;
    int desp1, desp2;

    if (leer_inodo(ninodo, &inodo) == FALLO){
        perror(RED "mi_read_f(): error leyendo el inodo"RESET);
        return FALLO;
    }

    if ((inodo.permisos & 4) != 4) {
       perror(RED "No hay permisos de lectura\n" RESET);
       return FALLO;
    }

    if ((offset + nbytes) >= inodo.tamEnBytesLog){ 
        nbytes = inodo.tamEnBytesLog - offset;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned int bloqueFisico = traducir_bloque_inodo(&inodo, primerBL, 1);

    if (bloqueFisico == FALLO){ 
        perror(RED "mi_write_f(): error al pasar bloque físico a lógico\n" RESET);
        return FALLO;
    }

    char unsigned bufBloque[BLOCKSIZE];
    if (bread(bloqueFisico, bufBloque) == FALLO){
        perror(RED "mi_write_f(): error al leer el bloque físico\n" RESET);
        return FALLO;
    }

    unsigned int bytesLeidos;

    if (primerBL == ultimoBL) {
        bytesLeidos = nbytes;
    } else {
        bytesLeidos = BLOCKSIZE - desp1;
    }

    if (memcpy(bufBloque + desp1, buf_original, bytesLeidos) == NULL) {
        perror(RED "mi_write_f() error\n" RESET);
        return FALLO;
    }
 
    bytesEscribiendo = bwrite(bloqueFisico, bufBloque);
    if (bytesEscribiendo == FALLO) {
        perror(RED "mi_write_f(): error al escribir bloques\n" RESET);
        return FALLO;
    }

    if (primerBL == ultimoBL) {
        bytesEscritos += nbytes;
    } else { // primerBL < ultimoBL
        bytesEscritos += bytesEscribiendo - desp1;
    }
    
    if (primerBL < ultimoBL){
        for (int bloque = 1 + primerBL; bloque < ultimoBL; bloque++){
            bloqueFisico = traducir_bloque_inodo(&inodo, bloque, 1);
            if (bloqueFisico == FALLO) {
                perror(RED "mi_write_f(): error al traducir bloque inodo\n" RESET);
                return FALLO;
            }

            bytesEscribiendo = bwrite(bloqueFisico, buf_original + (BLOCKSIZE - desp1) + (bloque - primerBL - 1) * BLOCKSIZE);
            
            if (bytesEscribiendo == FALLO) {
                perror(RED "mi_write_f(): error al escribir bloques\n" RESET);
                return FALLO;
            }

            bytesEscritos += bytesEscribiendo;
        }

        bloqueFisico = traducir_bloque_inodo(&inodo, ultimoBL, 1);

        if (bloqueFisico == FALLO) {
            perror(RED "mi_write_f: Error traduciendo el último bloque del inodo\n" RESET);
            return FALLO;
        }

        if (bread(bloqueFisico, bufBloque) == FALLO) {
            perror(RED "mi_write_f(): Error leyendo último bloque\n" RESET);
            return FALLO;
        }
        
        if(memcpy(bufBloque, buf_original + (nbytes - desp2 - 1), desp2 + 1) == NULL) {
            perror(RED "mi_write_f(): Error\n" RESET);
            return FALLO;
        }

        bytesEscribiendo = bwrite(bloqueFisico, bufBloque);

        if (bytesEscribiendo == FALLO) {
            perror(RED "mi_write_f(): error al escribir último bloque\n" RESET);
            return FALLO;
        }

        bytesEscritos += desp2 + 1;
    }

    if ((offset + nbytes) > inodo.tamEnBytesLog) { 
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO){
        fprintf(stderr, RED "mi_write_f(): error al escribir inodo %i" RESET, ninodo);
        return FALLO;
    }

    return bytesEscritos;
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
    if(leer_inodo(ninodo, &inodo) == -1){
        return FALLO;
    }

    // Rellenar la estructura p_stat
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.mtime;
    p_stat->mtime = inodo.ctime;
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
    if(escribir_inodo(ninodo, &inodo)==-1){
        return FALLO;
    }
    return EXITO;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    int primerBL, bloquesLiberados;
    if (leer_inodo(ninodo, &inodo) == FALLO){
        perror(RED "mi_truncar_f(): error al leer el inodo\n" RESET);
        return FALLO;
    }
    if ((inodo.permisos & 2) != 2){
        perror(RED "mi_truncar_f(): no hay permisos de escritura\n" RESET);
        return FALLO;
    }
    if (nbytes > inodo.tamEnBytesLog){
        perror(RED "mi_truncar_f(): nbytes mayor que el tamaño lógico\n" RESET);
        return FALLO;
    }
    if (nbytes % BLOCKSIZE == 0) {
        primerBL = nbytes / BLOCKSIZE;
    } else {
        primerBL = nbytes / BLOCKSIZE + 1;
    }
    bloquesLiberados = liberar_bloques_inodo(primerBL, &inodo);
    if (bloquesLiberados == FALLO){
        perror(RED "mi_truncar_f(): error al liberar bloques\n" RESET);
        return FALLO;
    }
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= bloquesLiberados;
    if (escribir_inodo(ninodo, &inodo) == FALLO){
        perror(RED "mi_truncar_f(): error al escribir el inodo\n" RESET);
        return FALLO;
    }
    return bloquesLiberados;
}

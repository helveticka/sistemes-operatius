#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){

}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){

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

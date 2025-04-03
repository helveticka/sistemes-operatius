/**
 * @file escribir.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "ficheros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define NUM_OFFSETS 5
/**
 * @brief Función principal que escribe en un inodo de un fichero
 * @param argc Cantidad de argumentos
 * @param argv Argumentos
 * @return EXITO si no hay errores, FALLO en caso contrario
 */
int main(int argc, char *argv[]) {
    unsigned int offsets[NUM_OFFSETS] = {9000, 209000, 30725000, 409605000, 480000000};
    if (argc != 4) {
        fprintf(stderr, RED"Sintaxis: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n"RESET);
        fprintf(stderr, RED"Offsets: "RESET);
        // Mostrar los offsets
        for (int i = 0; i < NUM_OFFSETS; i++) {
            fprintf(stderr, RED"%u"RESET, offsets[i]);
            if (i < NUM_OFFSETS - 1) {
                fprintf(stderr, RED", "RESET);
            }
        }
        fprintf(stderr, RED"\nSi diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n"RESET);
        return FALLO;
    }
    char *nombre_dispositivo = argv[1];
    char *texto = argv[2];
    int diferentes_inodos = atoi(argv[3]);
    unsigned int ninodo;
    int bytes_escritos, total_bytes = strlen(texto);
    printf("longitud texto: %d\n", total_bytes);
    // Montar el sistema de archivos
    if (bmount(nombre_dispositivo) == FALLO) {
        fprintf(stderr, RED"Error montando el dispositivo virtual en ./escribir"RESET);
        return FALLO;
    }
    // Reservar inodo
    for (int i = 0; i < NUM_OFFSETS; i++) {
        if (diferentes_inodos || i == 0) {
            ninodo = reservar_inodo('f', 6);
            if (ninodo == FALLO) {  
                fprintf(stderr, RED "Error al reservar inodo en ./escribir\n" RESET);  
                bumount();  
                return FALLO; 
            }
        }
        printf("\nNº inodo reservado: %d\n", ninodo);
        printf("offset: %u\n", offsets[i]);
        // Escribir en el inodo
        bytes_escritos = mi_write_f(ninodo, texto, offsets[i], total_bytes);
        if (bytes_escritos < 0) {
            fprintf(stderr, RED"Error en la escritura\n"RESET);
            bumount();
            return FALLO;
        }
        printf("Bytes escritos: %d\n", bytes_escritos);
        // Obtener información del inodo
        struct STAT stat;
        if (mi_stat_f(ninodo, &stat) < 0) {
            fprintf(stderr, RED "Error obteniendo stat\n"RESET);
            bumount();
            return FALLO;
        }
        printf("stat.tamEnBytesLog=%d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados=%d\n", stat.numBloquesOcupados);
    }
    // Desmontar el sistema de archivos
    if (bumount() == FALLO) {
        fprintf(stderr, RED"Error desmontando el dispositivo\n"RESET);
        return FALLO;
    }
    return EXITO;
}
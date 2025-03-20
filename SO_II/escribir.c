#include "ficheros.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BLOCKSIZE 1024

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Sintaxis: %s <nombre_dispositivo> <texto> <diferentes_inodos>\n", argv[0]);
        return -1;
    }
    
    // Montar el dispositivo virtual
    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el dispositivo\n");
        return -1;
    }

    char *texto = argv[2];
    unsigned int diferentes_inodos = atoi(argv[3]);
    unsigned int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};
    int num_offsets = sizeof(offsets) / sizeof(offsets[0]);

    unsigned int ninodo;
    if (!diferentes_inodos) {
        ninodo = reservar_inodo('f', 6);
        if (ninodo == -1) {
            fprintf(stderr, "Error al reservar inodo\n");
            bumount();
            return -1;
        }
        printf("Inodo reservado: %u\n", ninodo);
    }

    for (int i = 0; i < num_offsets; i++) {
        if (diferentes_inodos) {
            ninodo = reservar_inodo('f', 6);
            if (ninodo == -1) {
                fprintf(stderr, "Error al reservar inodo\n");
                bumount();
                return -1;
            }
            printf("Inodo reservado: %u\n", ninodo);
        }

        unsigned int offset = offsets[i];
        unsigned int nbytes = strlen(texto);
        char buf_original[nbytes];
        memset(buf_original, 0, nbytes);
        strcpy(buf_original, texto);

        // Escribir en el inodo
        int bytes_escritos = mi_write_f(ninodo, buf_original, offset, nbytes);
        if (bytes_escritos < 0) {
            fprintf(stderr, "Error al escribir en el inodo %u\n", ninodo);
            bumount();
            return -1;
        }
        printf("Bytes escritos en el inodo %u: %d\n", ninodo, bytes_escritos);

        // Leer para verificar
        char buf_lectura[nbytes];
        memset(buf_lectura, 0, nbytes);
        int bytes_leidos = mi_read_f(ninodo, buf_lectura, offset, nbytes);
        if (bytes_leidos < 0) {
            fprintf(stderr, "Error al leer del inodo %u\n", ninodo);
            bumount();
            return -1;
        }
        write(1, buf_lectura, bytes_leidos);
        printf("\n");

        // Obtener información del inodo
        struct STAT stat;
        if (mi_stat_f(ninodo, &stat) == -1) {
            fprintf(stderr, "Error al obtener stat del inodo %u\n", ninodo);
            bumount();
            return -1;
        }
        printf("Tamaño lógico: %u bytes, Bloques ocupados: %u\n", stat.tamEnBytesLog, stat.numBloquesOcupados);
    }

    // Desmontar el dispositivo
    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el dispositivo\n");
        return -1;
    }

    return 0;
}
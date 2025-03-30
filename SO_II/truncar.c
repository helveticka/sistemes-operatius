#include "ficheros.h"

int main(int argc, char **argv) {
    struct STAT stat;
    const char *nombre_dispositivo = argv[1];
    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);
    int bloques_liberados;

    // Validación de sintaxis
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo> <ninodo> <nbytes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Montar el dispositivo virtual
    if (bmount(nombre_dispositivo) == -1) {
        fprintf(stderr, "Error montando el dispositivo virtual\n");
        return EXIT_FAILURE;
    }
    
    if (nbytes == 0) {
        bloques_liberados = liberar_inodo(ninodo);
    } else {
        bloques_liberados = mi_truncar_f(ninodo, nbytes);
    }

    if (bloques_liberados < 0) {
        fprintf(stderr, "Error al truncar o liberar el inodo\n");
        bumount();
        return EXIT_FAILURE;
    }

    if (mi_stat_f(ninodo, &stat) == -1) {
        fprintf(stderr, "Error obteniendo información del inodo\n");
        bumount();
        return EXIT_FAILURE;
    }

    printf("\nDATOS INODO %d:\n", ninodo);
    printf("tipo=%c\n", stat.tipo);
    printf("permisos=%d\n", stat.permisos);
    printf("atime: %s", ctime(&stat.atime));
    printf("mtime: %s", ctime(&stat.mtime));
    printf("ctime: %s", ctime(&stat.ctime));
    printf("btime: %s", ctime(&stat.ctime));
    printf("nlinks=%d\n", stat.nlinks);
    printf("tamEnBytesLog=%d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados=%d\n", stat.numBloquesOcupados);

    // Desmontar el dispositivo virtual
    if (bumount() == -1) {
        fprintf(stderr, "Error desmontando el dispositivo virtual\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
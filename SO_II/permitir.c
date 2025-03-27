    #include "ficheros.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, RED"Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n" RESET);
        return FALLO;
    }

    // Obtener los argumentos
    char *nombre_dispositivo = argv[1];
    unsigned int ninodo = atoi(argv[2]);
    unsigned char permisos = atoi(argv[3]);

    // Montar el dispositivo
    if (bmount(nombre_dispositivo) == -1) {
        return FALLO;
    }

    // Cambiar permisos
    if (mi_chmod_f(ninodo, permisos) == -1) {
        bumount();
        return FALLO;
    }

    // Desmontar el dispositivo
    if (bumount() == -1) {
        return FALLO;
    }

    return EXITO;
}
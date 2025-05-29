#include "verificacion.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: verificacion <nombre_dispositivo> <directorio_simulación>\n"RESET);
        return FALLO;
    }
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED"Error al montar el dispositivo en ./verificacion\n"RESET);
        return FALLO;
    }
}
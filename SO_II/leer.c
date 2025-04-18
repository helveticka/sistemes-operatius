/**
 * @file leer.c
 * @author Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "ficheros.h"
/**
 * @brief Función principal que lee un fichero
 * @param argc Cantidad de argumentos
 * @param argv Argumentos
 * @return EXITO si no hay errores, FALLO en caso contrario
 */
int main(int argc, char **argv) {
    struct STAT p_stat;
    int ninodo, leidos, total_leidos = 0, offset = 0, tambuffer = 1500;
    char *path, buffer_texto[tambuffer];
    // Comprobamos que el número de argumentos sea el correcto
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: ./leer <nombre_dispositivo> <ninodo>\n" RESET);
        return FALLO;
    }
    // Comprobamos que el argumento de 'ninodo' sea un número entero
    ninodo = atoi(argv[2]);
    if (ninodo < 0) {
        fprintf(stderr, RED "ERROR: La entrada de 'ninodo' no es válida\n" RESET);
        return FALLO;
    }
    // Montamos el dispositivo virtual
    path = argv[1];
    if (bmount(path) == FALLO) {
        fprintf(stderr, RED "Error al montar el dispositivo virtual en ./leer" RESET);
        return FALLO;
    }
    // Leemos el inodo
    memset(buffer_texto, 0, tambuffer);
    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    // Leemos el contenido del inodo
    while (leidos > 0) {
        write(1, buffer_texto, leidos);
        total_leidos += leidos;
        offset += tambuffer;
 
        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
        if (leidos == FALLO) {
            fprintf(stderr, RED "Error en mi_read_f en ./leer" RESET);
            return FALLO;
        }
    }
    // Obtenemos los metadatos del inodo
    if (mi_stat_f(ninodo, &p_stat) == FALLO) {
        fprintf(stderr, RED "Error en mi_stat_f en ./leer" RESET);
        return FALLO;
    }
#if DEBUGN5 || ENTREGA_1
    fprintf(stderr, "total_leidos: %d\n", total_leidos);
    fprintf(stderr, "tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
#endif
    if (bumount() == FALLO){
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual en ./leer" RESET);
        return FALLO;
    }
    return EXITO;
}
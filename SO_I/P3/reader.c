// Autores: Xavier Campos, Pedro Félix, Harpo Joan
#include "my_lib.h"
/**
 * Función principal que lee los datos de la pila almacenados en un fichero
 argc: número de argumentos
 *argv: argumentos del terminal
 return: 0 si todo ha ido bien, 1 si ha habido un error
 */
int main(int argc, char *argv[]) {
    // Inicialización de variables
    static struct my_stack *stack;
    int sum = 0;
    int min = INT_MAX;
    int max = 0;
    int items;
    int *data;
    // Se comprueba que se ha pasado un argumento
    if (argv[1] == NULL) {
        fprintf(stderr, ROJO "USAGE: ./reader <stack_file>\n" RESET);
        return EXIT_FAILURE;
    }
    // Se lee la pila del fichero
    if ((stack = my_stack_read(argv[1])) == NULL) {
        fprintf(stderr, ROJO "Couldn't open stack file %s\n" RESET, argv[1]);
        return EXIT_FAILURE;
    }
    // Se obtiene la longitud de la pila
    items = my_stack_len(stack);
    fprintf(stderr, "Stack length: %d\n", items);
    // Se recorren los elementos de la pila
    for (int i = 0; ((stack -> top != NULL) && (i < NUM_THREADS)); i++) {  
        data = my_stack_pop(stack);
        sum += *data;
        // Se actualizan los valores de min y max
        if (*data < min) min = *data;
        if (*data > max) max = *data;
        fprintf(stderr, "%d\n", *data);
    }
    // Se muestran los resultados
    if (items < NUM_THREADS) {
        fprintf(stderr, "\nItems: %d", items);
    } else {
        fprintf(stderr, "\nItems: %d", NUM_THREADS);
    }
    fprintf(stderr, NEGRITA" Sum:"RESET);
    fprintf(stderr, AZUL" %d"RESET, sum);
    fprintf(stderr, " Min: %d Max: %d", min, max);
    fprintf(stderr, NEGRITA" Average:"RESET);
    fprintf(stderr, AZUL" %d\n"RESET, (sum / NUM_THREADS));
    my_stack_purge(stack);
    return EXIT_SUCCESS;
}
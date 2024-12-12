// Autores: Xavier Campos, Pedro Felix, Harpo Joan

#include "my_lib.h"

int main(int argc, char *argv[]) {
    struct my_stack *stack;
    int sum = 0;
    int min = INT_MAX;
    int max = 0;
    int items;
    int *data;

    if (argv[1] == NULL) {
        fprintf(stderr, ROJO "USAGE: ./reader <stack_file>\n" RESET);
        return EXIT_FAILURE;
    }
    if ((stack = my_stack_read(argv[1])) == NULL) {
        fprintf(stderr, ROJO "Couldn't open stack file %s\n" RESET, argv[1]);
        return EXIT_FAILURE;
    }
    items = my_stack_len(stack);
    fprintf(stderr, "Stack length: %d\n", items);
    for (int i = 0; ((stack -> top != NULL) && (i < NUM_THREADS)); i++) {  
        data = my_stack_pop(stack);
        sum += *data;
        if (*data < min) min = *data;
        if (*data > max) max = *data;
        fprintf(stderr, "%d\n", *data);
    }
    fprintf(stderr, "\nItems: %d Sum: %d Min: %d Max: %d Average: %d\n", items, sum, min, max, (sum / NUM_THREADS));
    my_stack_purge(stack);
    return EXIT_SUCCESS;
}



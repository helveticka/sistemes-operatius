// Autores: Xavier Campos, Pedro Felix, Harpo Joan
#define _POSIX_C_SOURCE 200112L
#include "my_lib.h"

// Puntero a la pila global y un mutex para sincronización de hilos
static struct my_stack *stack; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Lee o inicializa una pila, crea hilos que operan concurrentemente sobre la pila y guarda el estado final.
 * 
 * argc: Número de argumentos proporcionados por el usuario.
 * argv: Argumentos, el primero debe ser el archivo de la pila.
 * return: int EXIT_SUCCESS si todo es exitoso, EXIT_FAILURE en caso de errores.
 */
int main (int argc, char *argv[]) {
    // Verifica que se haya pasado un archivo como argumento.
    if (argv[1] == NULL) {
        fprintf(stderr, ROJO "USAGE: ./stack_counters <stack_file>\n" RESET);
        return EXIT_FAILURE;
    }

    // Imprime el número de hilos y el número de iteraciones por hilo.
    fprintf(stderr, "Threads: %d, Iterations: %d\n", NUM_THREADS, N);

    // Intenta leer la pila desde el archivo proporcionado.
    if ((stack = my_stack_read(argv[1])) == NULL) {
        // Si no existe la pila en el archivo, inicializa una nueva.
        if ((stack = my_stack_init(sizeof(int))) == NULL) {
            return EXIT_FAILURE;
        }
        fprintf(stderr, "stack->size: %d\n", stack->size);

        // Agrega elementos iniciales a la pila.
        for (int i = 0; i < NUM_THREADS; i++) {
            int *data = malloc(sizeof(int));  // Reserva memoria para un entero.
            if (data == NULL) {
                perror("malloc()");  // Manejo de errores en la asignación.
                return EXIT_FAILURE;
            }
            *data = 0;               // Inicializa el dato en 0.
            my_stack_push(stack, data);  // Empuja el dato a la pila.
        }

    } else if (my_stack_len(stack) < NUM_THREADS) {
        // Si la pila es más pequeña que el número de hilos, la ajusta.
        int items = my_stack_len(stack);
        int added = NUM_THREADS - items;
        for (int i = 0; i < added; i++) {
            int *data = malloc(sizeof(int));
            if (data == NULL) {
                perror("malloc()");
                return EXIT_FAILURE;
            }
            *data = 0;
            my_stack_push(stack, data);
        }
    }

    // Creación y manejo de hilos
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Guarda la pila en el archivo
    int items;
    if ((items = my_stack_write(stack, argv[1])) < 0) {
        perror("my_stack_write()");
        return EXIT_FAILURE;
    }

    // Libera recursos y finaliza
    fprintf(stderr, "Released bytes: %d\n", my_stack_purge(stack));
    pthread_exit(EXIT_SUCCESS);   
    return EXIT_SUCCESS;
}

/**
 * Los hilos extraen, modifican y vuelven a insertar elementos de la pila.
 * 
 * ptr: No se utiliza en este caso.
 * return: void* Siempre NULL.
 */
void *worker(void *ptr) {
    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(&mutex);  // Bloquea el mutex para acceder a la pila.
        int *data = my_stack_pop(stack);  // Extrae un elemento de la pila.
        pthread_mutex_unlock(&mutex);    // Libera el mutex.

        (*data)++;  // Incrementa el valor del elemento.

        pthread_mutex_lock(&mutex);  // Vuelve a bloquear el mutex.
        my_stack_push(stack, data);  // Inserta el elemento de nuevo en la pila.
        pthread_mutex_unlock(&mutex);  // Libera el mutex.
    }
    pthread_exit(NULL);
}

/**
 * Imprime todos los elementos de la pila.
 * 
 * stack: Puntero a la estructura de la pila.
 */
void imprimir_pila(struct my_stack *stack) {
    struct my_stack_node *node = stack->top;  // Apunta al nodo superior de la pila.
    while (node != NULL) {
        int *element = (int*)node->data;  // Obtiene el dato del nodo actual.
        fprintf(stderr, "%d\n", *element);  // Imprime el dato.
        node = node->next;  // Avanza al siguiente nodo.
    }
}

/**
 * Suspende la ejecución por un tiempo específico.
 * 
 * msec: Tiempo en milisegundos para suspender.
 */
void sleep_milisegundos(unsigned int msec) {
    struct timespec req;
    req.tv_sec = msec / 1000;               // Convierte milisegundos a segundos.
    req.tv_nsec = (msec % 1000) * 1000000;  // Convierte el resto a nanosegundos.
    nanosleep(&req, NULL);                  // Suspende la ejecución.
}
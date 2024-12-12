// Autores: Xavier Campos, Pedro Felix, Harpo Joan

#include "my_lib.h"
#define DEBUG 0

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main (int argc, char *argv[]) {

    if (argv[1] == NULL) {
        fprintf(stderr, ROJO "USAGE: ./stack_counters <stack_file>\n" RESET);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "Threads: %d, Iterations: %d\n", NUM_THREADS, N);
    if ((stack = my_stack_read(argv[1])) == NULL) {
        if ((stack = my_stack_init(sizeof(int))) == NULL) {
            return EXIT_FAILURE;
        }
        fprintf(stderr, "stack->size: %d\n", stack -> size);
#if DEBUG
        fprintf(stderr, "original stack length: %d\n", my_stack_len(stack));
#else
        fprintf(stderr, "initial stack length: %d\n", my_stack_len(stack));
        fprintf(stderr, "initial stack content:\n");
#endif
        for (int i = 0; i < NUM_THREADS; i++) {
            int *data = malloc(sizeof(int));
            if (data == NULL) {
                perror("malloc() error");
                return EXIT_FAILURE;
            }
            *data = 0;
            my_stack_push(stack, data);
        }
#if !DEBUG
        fprintf(stderr, "stack content for treatment:\n");
        print_stack(stack);
#endif
        fprintf(stderr, "stack length: %d\n\n", my_stack_len(stack));
    } else if (my_stack_len(stack) < NUM_THREADS) {
        int items = my_stack_len(stack);
        int added = NUM_THREADS - items;
        fprintf(stderr, "initial stack length: %d\n", items);
        fprintf(stderr, "original stack content:\n");
        print_stack(stack);
        fprintf(stderr, "\nNumber of elements added to inital stack: %d\n", added);
        for (int i = 0; i < added; i++) {
            int *data = malloc(sizeof(int));
            if (data == NULL) {
                perror("malloc() error");
                return EXIT_FAILURE;
            }
            *data = 0;
            my_stack_push(stack, data);
        }
        fprintf(stderr, "stack content for treatment:\n");
        print_stack(stack);
        fprintf(stderr, "new stack length: %d\n\n", my_stack_len(stack));
    } else {
        fprintf(stderr, "original stack content:\n");
        print_stack(stack);
        fprintf(stderr, "original stack length: %d\n\n", my_stack_len(stack));
    }

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
#if DEBUG
        fprintf(stderr, ORANGE_TEXT "%d) Thread %lu created\n" RESET, i, threads[i]);
#else
        fprintf(stderr, "%d) Thread %p created\n", i, (void *)threads[i]);
#endif
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
#if !DEBUG
    fprintf(stderr, "\nstack content after threads iterations:\n");
    print_stack(stack);
    fprintf(stderr, "stack length: %d \n", my_stack_len(stack));
#endif
    int items;
    if ((items = my_stack_write(stack, argv[1])) < 0) {
        return EXIT_FAILURE;
    }
    fprintf(stderr, "\nWritten elements from stack to file: %d\n", items);
    fprintf(stderr, "Released bytes: %d\n", my_stack_purge(stack));
    fprintf(stderr, "Bye from main\n");
    pthread_exit(EXIT_SUCCESS);   
    return EXIT_SUCCESS;
}

void *worker(void *ptr) {
    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(&mutex);
        int *data = my_stack_pop(stack);
        pthread_mutex_unlock(&mutex);
        (*data)++;
        pthread_mutex_lock(&mutex);
        my_stack_push(stack, data);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void print_stack(struct my_stack *stack) {
    void *data = malloc(sizeof(int));
    struct my_stack_node *node = stack -> top;
    while (node != NULL) {
        data = node -> data;
        fprintf(stderr, "%d\n", *((int*)data));
        node = node -> next;
    }
}
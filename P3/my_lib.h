/* lib.h librería con las funciones equivalentes a las
de <string.h> y las funciones y estructuras para el
manejo de una pila */

#include <stdio.h>     /* para printf en depurarión */
#include <string.h>    /* para funciones de strings  */
#include <stdlib.h>    /* Funciones malloc(), free(), y valor NULL */
#include <fcntl.h>     /* Modos de apertura de función open()*/
#include <sys/stat.h>  /* Permisos función open() */
#include <sys/types.h> /* Definiciones de tipos de datos como size_t*/
#include <unistd.h>    /* Funciones read(), write(), close()*/
#include <errno.h>     /* COntrol de errores (errno) */
#include <pthread.h>   /* Hilos */
#include <limits.h>

//declaraciones funciones libreria string
size_t my_strlen(const char *str);
int my_strcmp(const char *str1, const char *str2);
char *my_strcpy(char *dest, const char *src);
char *my_strncpy(char *dest, const char *src, size_t n);
char *my_strcat(char *dest, const char *src);
char *my_strchr(const char *s, int c);

// char *my_strncat(char *dest, const char *src, size_t n);

//structuras para gestor de pila
struct my_stack_node {      // nodo de la pila (elemento)
    void *data;
    struct my_stack_node *next;
};

struct my_stack {   // pila
    int size;       // tamaño de data, nos lo pasarán por parámetro
    struct my_stack_node *top;  // apunta al nodo de la parte superior
};  

//declaraciones funciones gestor de pila
struct my_stack *my_stack_init(int size);
int my_stack_push(struct my_stack *stack, void *data);
void *my_stack_pop(struct my_stack *stack);
int my_stack_len(struct my_stack *stack);
int my_stack_purge(struct my_stack *stack); 
struct my_stack *my_stack_read(char *filename);
int my_stack_write(struct my_stack *stack, char *filename);
    
 // Códigos de color
#define RESET "\033[0m"
#define NEGRO "\x1b[30m"
#define GRIS "\x1b[94m"
#define ROJO "\x1b[31m"
#define VERDE "\x1b[32m"
#define AMARILLO "\x1b[33m"
#define AZUL "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define BLANCO "\x1b[97m"
#define NARANJA "\x1B[38;2;255;128;0m"
#define NEGRITA "\x1b[1m"

// variables y constantes
static struct my_stack *stack;
#define NUM_THREADS 10
#define N 1000000
#define DEBUG 0

// declaraciones funciones gestión de hilos
void *worker(void *ptr);
void imprimir_pila(struct my_stack *stack);
void sleep_milisegundos(unsigned int msec);
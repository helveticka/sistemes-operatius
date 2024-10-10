// Autores: Xavier Campos, Pedro Felix, Harpo Joan

#include "my_lib.h"

void main(){
  char cadena[]="Hola";
  printf("my_strlen(\"%s\"): %ld\n", cadena, my_strlen(cadena));
  return;
}

size_t my_strlen(const char *str) {
    size_t len = 0;
    int i = 0;
    while (str[i]) {
        i++;
        len++;
    }
    return len;
}

int my_strcmp(const char *str1, const char *str2) {

}

char *my_strcpy(char *dest, const char *src) {
    int src_size = my_strlen(src);
    int i;
    for (i = 0; i < src_size; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}

/*
Copia n caracteres de la cadena src a dest.
*dest: cadena destino
*src: cadena fuente
n: numero de caracteres a copiar
*/
char *my_strncpy(char *dest, const char *src, size_t n) {
    int src_size = my_strlen(src);
    if (n > src_size) {
        int i;
        for (i = 0; i < src_size; i++) {
            dest[i] = src[i];
        }
        for (; i <= strlen(dest); i++) {
            dest[i] = '\0';
        }
    } else {
        for (int i = 0; i < n; i++) {
            dest[i] = src[i];
        }
    }
    return dest;
}

char *my_strcat(char *dest, const char *src) {
    int src_size = my_strlen(src);
    int dest_size = my_strlen(dest);
    int i;
    for (i = 0; i < dest_size + src_size - 1; i++) {
        dest[src_size + i] = src[i];
    }
    dest[src_size + i] = '\0';
    return dest;
}

char *my_strchr(const char *str, int c) {

}

/*
Inicializa una struct de tipo pila.
size: tamano de la pila
return: devuelve un puntero a la pila inicializada
*/
struct my_stack *my_stack_init (int size) {
    struct my_stack *stack = malloc(size);
    stack -> size = size;
    stack -> top = NULL;
    return stack;
}

int my_stack_push (struct my_stack *stack, void *data) {
    //TODO xavi
}

void *my_stack_pop (struct my_stack *stack) {
    //TODO pedro
}

int my_stack_len (struct my_stack *stack) {
    struct my_stack_node *aux_node = stack -> top; 
    int size = 0;
    while (aux_node != NULL) {    
        size++;
        aux_node = aux_node -> next;        
    }
    return size;
}

int my_stack_purge (struct my_stack *stack) {
    //TODO xavi
}

int my_stack_write (struct my_stack *stack, char *filename) {
    //TODO harpo
}

struct my_stack *my_stack_read (char *filename) {
    //TODO pedro
}
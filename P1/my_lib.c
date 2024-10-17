// Autores: Xavier Campos, Pedro Felix, Harpo Joan

#include "my_lib.h"
#include <stdbool.h>

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
    bool cmp = true;
    int i = 0;
    int diff = 0;
    while(cmp&&(str1[i]||str2[i])){
        diff = str1[i]-str2[i];
        if(diff!=0){
            cmp=false;
        }
        i++;
    }
    return diff;
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
    //Xavi
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

/*
Devuelve la longitud de la pila.
*stack: puntero a la pila
return: numero de nodos
*/
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

/*
Almacena los datos de la pila en un fichero
*stack: puntero a la pila
*filename: nombre del fichero
return: numero de elementos almacenados
*/
int my_stack_write (struct my_stack *stack, char *filename) {
    int fd;
    int data_bytes;
    int read_bytes = 0;
    const int BYTES = 4;
    void *data;
    struct my_stack *aux_stack;
    struct my_stack_node *aux_node;

    if (stack != NULL) {
        data_bytes = stack -> size;
        aux_stack = my_stack_init(stack -> size);
        aux_node = stack -> top;
        while (aux_node != NULL) {
            my_stack_push(aux_stack, aux_node -> data);
            aux_node = aux_node -> next;
        }
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd != -1) {
            if (write(fd, &data_bytes, BYTES) != -1 ) {
                data = my_stack_pop(aux_stack);
                while (data != NULL) {
                    if (write(fd, data, data_bytes) != -1) {
                        read_bytes++;
                    }
                    data = my_stack_pop(aux_stack);
                }
            }
            else {
                read_bytes = -1;
            }
            close(fd);
        }
        else {
            read_bytes = -1;
        }
    }
    return read_bytes;
}

struct my_stack *my_stack_read (char *filename) {
    //TODO pedro
}
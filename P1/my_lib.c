// Autores: Xavier Campos, Pedro Felix, Harpo Joan

#include "my_lib.h"
//#include <stdbool.h>


/*
Devuelve la longitud de una cadena.
*str: cadena de caracteres
return: longitud de la cadena
*/
size_t my_strlen(const char *str) {
    size_t len = 0;
    int i = 0;
    while (str[i]) {
        i++;
        len++;
    }
    return len;
}

/*
Compara dos cadenas carácter a carácter a nivel de codigo ASCII
*str1: primera cadena a comparar
*str2: segunda cadena a comparar
return: diferencia numérica obtenida al restar los codigos ASCII de str2 a str1
*/
int my_strcmp(const char *str1, const char *str2) {
    int cmp = 1;
    int i = 0;
    int diff = 0;
    while(cmp&&(str1[i]||str2[i])){
        diff = str1[i]-str2[i];
        if(diff!=0){
            cmp=0;
        }
        i++;
    }
    return diff;
}

/*
Copia una cadena de caracteres de src a dest.
*dest: cadena destino
*src: cadena fuente
return: puntero a la cadena destino
*/
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

/*
Concatena la cadena src al final de dest.
*dest: cadena destino
*src: cadena fuente
return: puntero a la cadena destino
*/
char *my_strcat(char *dest, const char *src) {
    int src_size = my_strlen(src);
    int dest_size = my_strlen(dest);
    int i;
    for (i = 0; i < dest_size + src_size - 1; i++) {
        dest[src_size + i + 1] = src[i];
    }
    dest[src_size + i + 1] = '\0';
    return dest;
}

/*
Busca la primera ocurrencia de un carácter dado en una cadena
*str: cadena de caracteres
c: carácter a buscar
return: puntero que apunta a la primera ocurrencia de c
*/
char *my_strchr(const char *str, int c) {
    while(*str != '\0'){
        if(*str == (char)c){
            return (char *)str;
        }
        str++;
    }
    return NULL;
}

/*
Inicializa una struct de tipo pila.
size: tamano de la pila
return: devuelve un puntero a la pila inicializada
*/
struct my_stack *my_stack_init(int size) {
    struct my_stack *stack = malloc(size);
    stack -> size = size;
    stack -> top = NULL;
    return stack;
}

/*
Inserta un nuevo nodo en los elementos de la pila
*stack: puntero a la pila
*data: dato a insertar en la pila
return: 0 si ha funcionado, -1 si ha habido error
*/
int my_stack_push(struct my_stack *stack, void *data) {
    struct my_stack_node *new_node = malloc(sizeof(struct my_stack_node));
    if (new_node == NULL) {
        return -1;
    }
    new_node->data = data;
    new_node->next = stack->top;
    stack->top = new_node;
    return 0;
}

/*
Elimina el nodo superior de la pila.
*stack: puntero a la pila.
return: puntero a los datos del elemento eliminado 
o NULL si la pila está vacía.
*/
void *my_stack_pop(struct my_stack *stack) {
    if(stack -> top == NULL){
        return NULL;
    }
    void *data = stack -> top -> data;
    stack -> top = stack -> top -> next;
    return data;
}

/*
Devuelve la longitud de la pila.
*stack: puntero a la pila
return: numero de nodos
*/
int my_stack_len(struct my_stack *stack) {
    struct my_stack_node *aux_node = stack -> top; 
    int size = 0;
    while (aux_node != NULL) {    
        size++;
        aux_node = aux_node -> next;        
    }
    return size;
}

/*
Recorre la pila liberando la memoria reservada
para cada uno de los datos y para cada nodo
*stack: puntero a la pila
return: número de bytes liberados
*/
int my_stack_purge(struct my_stack *stack) {
    struct my_stack_node *current = stack->top;
    struct my_stack_node *next;
    int total_bytes = 0;
    while (current != NULL) {
        next = current->next;
        total_bytes += stack->size;
        free(current->data);
        total_bytes += sizeof(*current);
        free(current);
        current = next;
    }
    total_bytes += sizeof(*stack);
    free(stack);
    return total_bytes;
}

/*
Almacena los datos de la pila en un fichero
*stack: puntero a la pila
*filename: nombre del fichero
return: numero de elementos almacenados
*/
int my_stack_write(struct my_stack *stack, char *filename) {
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
        if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == -1) {
            perror("my_stack_write() error");
            read_bytes = -1;
        } else {
            if (write(fd, &data_bytes, BYTES) == -1) {
                perror("my_stack_write() error");
                read_bytes = -1;
            } else {
                data = my_stack_pop(aux_stack);
                while (data != NULL) {
                    if (write(fd, data, data_bytes) != -1) {
                        read_bytes++;
                    }
                    data = my_stack_pop(aux_stack);
                }
            }
            close(fd);
        }
    }
    return read_bytes;
}

/*
Lee los datos de la pila almacenados en el fichero
y reconstruye la pila en memoria principal.
*filename: nombre del fichero.
return: puntero a la pila creada o NULL si hubo error.
*/
struct my_stack *my_stack_read(char *filename) {
    int fd;
    struct my_stack *stack;
    void *data;
    int size = sizeof(int);
    

    if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("my_stack_read() error");
        return NULL;
    }

    if (read(fd, &size, size) == -1) {
        perror("my_stack_read() error");
        return NULL;
    }
    
    stack = my_stack_init(size);
    data = malloc(size);
    if(!data){
        perror("malloc() error");
        close(fd);
        return NULL;
    }

    while (read(fd, data, size)>0){
        my_stack_push(stack,data);
        data = malloc(size);
        if(!data){
            perror("malloc() error");
            close(fd);
            return NULL;
        }
        
    }
    free(data);
    close(fd);
    return stack;
}
// Autores: 

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

}

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

}

char *my_strchr(const char *str, int c) {

}
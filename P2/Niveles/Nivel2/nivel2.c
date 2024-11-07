// Autores: Xavier Campos, Pedro Felix, Harpo Joan

#include "nivel2.h"

int main() {
    while (1) {
        if (read_line(line)) {
            execute_line(line);
        }
    }
    return EXIT_SUCCESS;
}

/**
 Imprime el prompt y lee una linea de consola.
 *line: puntero donde se almacenará la linea leída.
 return: puntero a la línea leída.
 */
char *read_line(char *line) {
    print_prompt();
    // Lee una línea desde stdin
    if(fgets(line,ARGS_SIZE,stdin) != NULL){
        // Sustituye el carácter '\n' por '\0'
        if(strlen(line) > 0 && line[strlen(line) - 1] == '\n'){
            line[strlen(line) - 1] = '\0';
        }
        return line;
    } else{
        // Verifica si fue por Ctrl + D
        if(feof(stdin)){
#if DEBUGN1
            printf("\rAdiós\n");
#endif
            exit(0);
        }

    }
    return line;
}

int execute_line(char *line) {
    char *args[ARGS_SIZE];
    if(parse_args(args, line) > 0){
        check_internal(args);
    }
    return 0;
}

int parse_args(char **args, char *line) {
    const char *delimiters = " \t\n\r";
    char *token;
    unsigned int counter = 0;
    token = strtok(line,delimiters);
    while(token != NULL){
        if(token[0] == '#'){
#if DEBUGN1
            printf(GRIS "[parse_args()→ token %d: %s]\n" RESET, counter,token);
#endif
            token = NULL;
            args[counter] = token;
#if DEBUGN1
            printf(GRIS "[parse_args()→ token %d corregido: %s]\n" RESET, counter,token);
#endif
            counter++;
            return counter;
        }
        args[counter] = token;
#if DEBUGN1
        printf(GRIS "[parse_args()→ token %d: %s]\n" RESET, counter,token);
#endif
        counter++;
        token = strtok(NULL,delimiters);
    }
#if DEBUGN1
    printf(GRIS "[parse_args()→ token %d: %s]\n" RESET, counter, token);
#endif
    return counter;
}

int check_internal(char **args) {
    int command = 0;

    if (args == NULL || args[0] == NULL) {
        return 0;
    }

    if (!strcmp(args[0], "cd")) {
        internal_cd(args);
        command = 1;
    }

    if (!strcmp(args[0], "export")) {
        internal_export(args);
        command = 1;
    }

    if (!strcmp(args[0], "source")) {
        internal_source(args);
        command = 1;
    }

    if (!strcmp(args[0], "jobs")) {
        internal_jobs(args);
        command = 1;
    }

    if (!strcmp(args[0], "fg")) {
        internal_fg(args);
        command = 1;
    }

    if (!strcmp(args[0], "bg")) {
        internal_bg(args);
        command = 1;
    }

    if (!strcmp(args[0], "exit")) {
        printf("Bye Bye\n");
        exit(0);
        return EXIT_SUCCESS;
    }
    return command;
}

int internal_cd(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_cd()→ Esta función cambiará de directorio]\n" RESET);
#endif
    char *home_dir = getenv("HOME");      // Obtener el directorio HOME
    char current_dir[1024];           // Buffer para el directorio actual
    int result = 0;

    // Caso 1: Sin argumentos (ir al directorio HOME)
    if (args[1] == NULL) {
        if (home_dir == NULL) {
            fprintf(stderr, "Error: la variable HOME no está definida.\n");
            return -1;
        }
        result = chdir(home_dir);
    }
    // Caso 2: Un solo argumento (directorio especificado en args[1])
    else if (args[2] == NULL) {
        result = chdir(args[1]);
    }

    // Verificar si `chdir()` tuvo éxito
    if (result != 0) {
        // Si `chdir` falla, imprime el error y retorna -1
        perror("Error al cambiar de directorio");
        return -1;
    }

    // Obtener el directorio actual después del cambio
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("Error al obtener el directorio actual");
        return -1;
    }

    // Mostrar el directorio actual
    printf("Directorio actual: %s\n", current_dir);

    return 0;
}
int internal_export(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_export()→ Esta función asignará valores a variables cd de entorno]\n" RESET);
#endif
    // Verificar si el número de argumentos es adecuado (debe tener args[1])
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Uso: export NOMBRE=VALOR\n");
        return -1;
    }

    // Descomponer en tokens el argumento NOMBRE=VALOR
    char *name = strtok(args[1], "=");
    char *value = strtok(NULL, "=");

    // Verificar si el formato es correcto (debe haber un nombre y un valor)
    if (name == NULL || value == NULL) {
        fprintf(stderr, "Uso: export NOMBRE=VALOR\n");
        return -1;
    }

    // Mostrar el valor inicial de la variable de entorno
    char *initial_value = getenv(name);
    if (initial_value != NULL) {
        printf("Valor inicial de %s: %s\n", name, initial_value);
    } else {
        printf("La variable %s no estaba definida.\n", name);
    }

    // Asignar el nuevo valor a la variable de entorno
    if (setenv(name, value, 1) != 0) {
        perror("Error al establecer la variable de entorno");
        return -1;
    }

    // Mostrar el nuevo valor de la variable de entorno
    char *new_value = getenv(name);
    if (new_value != NULL) {
        printf("Nuevo valor de %s: %s\n", name, new_value);
    } else {
        fprintf(stderr, "Error: la variable %s no se estableció correctamente.\n", name);
        return -1;
    }

    return 0;
}

int internal_source(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_source()→ Esta función ejecutará un fichero de líneas de comandos]\n" RESET);
#endif
    return EXIT_SUCCESS;
} 

int internal_jobs(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n" RESET);
#endif
    return EXIT_SUCCESS;
}

int internal_fg(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_fg()→ Esta función traerá los procesos más recientes al primer plano]\n" RESET);
#endif
    return EXIT_SUCCESS;
}

int internal_bg(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_bg()→ Esta función mostrará los procesos parados o en segundo plano]\n" RESET);
#endif
    return EXIT_SUCCESS;
}

void print_prompt(void) {
    user = getenv("USER");
    char path[200];
    getcwd(path, sizeof(path));

    printf(NEGRITA AZUL "%s: " RESET, user);
    printf(NEGRITA "%s%c " RESET, path, PROMPT);
    fflush(stdout);
}
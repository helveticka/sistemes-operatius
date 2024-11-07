//Autores: 

#include "nivel1.h"

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
            printf("[parse_args()→ token %d: %s]\n", counter,token);
#endif
            token = NULL;
            args[counter] = token;
#if DEBUGN1
            printf("[parse_args()→ token %d corregido: %s]\n", counter,token);
#endif
            counter++;
            return counter;
        }
        args[counter] = token;
#if DEBUGN1
        printf("[parse_args()→ token %d: %s]\n", counter,token);
#endif
        counter++;
        token = strtok(NULL,delimiters);
    }
#if DEBUGN1
    printf("[parse_args()→ token %d: %s]\n", counter, token);
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
    fprintf(stderr, GRIS "[internal_cd() → Esta función cambiará de directorio]\n" RESET);
#endif
    return EXIT_SUCCESS;
}
int internal_export(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_export() → Esta función asignará valores a variables cd de entorno]\n" RESET);
#endif
    return EXIT_SUCCESS;
}

int internal_source(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_source() → Esta función ejecutará un fichero de líneas de comandos]\n" RESET);
#endif
    return EXIT_SUCCESS;
} 

int internal_jobs(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_jobs() → Esta función mostrará el PID de los procesos que no estén en foreground]\n" RESET);
#endif
    return EXIT_SUCCESS;
}

int internal_fg(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_fg() → Esta función traerá los procesos más recientes al primer plano]\n" RESET);
#endif
    return EXIT_SUCCESS;
}

int internal_bg(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_bg() → Esta función mostrará los procesos parados o en segundo plano]\n" RESET);
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
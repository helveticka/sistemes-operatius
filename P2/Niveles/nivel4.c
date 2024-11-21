// Autores: Xavier Campos, Pedro Felix, Harpo Joan
#include "nivel4.h"
/**
 * Método principal del archivo.
 */
int main(int argc, char *argv[]) {
    // Señales
    signal(SIGCHLD, reaper); 
    signal(SIGINT, ctrlc); 
    // Inicialización de la lista de trabajos
    jobs_list[FOREGROUND].pid = 0;
    jobs_list[FOREGROUND].estado = NINGUNO;
    memset(jobs_list[FOREGROUND].cmd, '\0', sizeof(jobs_list[FOREGROUND].cmd));
    strcpy(mi_shell,argv[0]);
    // Bucle para leer líneas y ejecutarlas
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
    fflush(stdout);
    memset(line, '\0', COMMAND_LINE_SIZE);
    char *ptr = fgets(line,COMMAND_LINE_SIZE,stdin);
    // Lee una línea desde stdin
    if(ptr != NULL){
        // Sustituye el carácter '\n' por '\0'
        line[strlen(line) - 1] = '\0';
    } else{
        // Verifica si fue por Ctrl + D
        if(feof(stdin)){
#if DEBUGN1
            fprintf(stderr, GRIS "\rAdiós\n" RESET);
#endif
            exit(EXIT_SUCCESS);
        }

    }
    return ptr;
}
/**
 Ejecuta una línea de comandos.
 *line: puntero a la línea de comandos.
 return: 0 si todo fue bien, -1 si hubo algún error.
 */
int execute_line(char *line) {
    char *args[ARGS_SIZE];
    int numArgs = parse_args(args, line);
    if (numArgs > 0) {

#if DEBUGN3 || DEBUGN4
        char command[COMMAND_LINE_SIZE] = "";
        for (size_t i = 0; i < numArgs; i++) {
            strcat(command, args[i]);
            if (i < numArgs - 1) {
                strcat(command, " ");
            }
        }
        command[COMMAND_LINE_SIZE - 1] = '\0';
#endif
        // Verificar si es un comando interno
        if (check_internal(args) == 0) {
            // Comando externo, crear proceso hijo
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error en fork");
                return -1;
            }
            // Proceso hijo
            if (pid == 0) { 
                // Asignar acción por defecto a la señal SIGCHLD
                signal(SIGCHLD, SIG_DFL);
                // Ignorar la señal SIGINT en el proceso hijo
                signal(SIGINT, SIG_IGN);
                // Ejecuta el comando externo
                if (execvp(args[0], args) < 0) {
                    // Si execvp falla, muestra el error y termina
                    fprintf(stderr, ROJO "%s: ", args[0]);
                    perror("");
                    printf(RESET);
                    exit(-1);
                }
                exit(0);
            // Proceso padre (minishell)
            } else if (pid > 0) { 
                // Actualizar jobs_list para el proceso en foreground
                jobs_list[0].pid = pid;
                strncpy(jobs_list[0].cmd, command, COMMAND_LINE_SIZE - 1);
                jobs_list[0].cmd[COMMAND_LINE_SIZE - 1] = '\0';
                jobs_list[0].estado = 'E'; // EJECUTANDOSE
#if DEBUGN3 || DEBUGN4
                // Imprimir información de depuración
                printf(GRIS"[execute_line()→ PID padre: %d (%s)]\n"RESET, getpid(), mi_shell);
                printf(GRIS"[execute_line()→ PID hijo: %d (%s)]\n"RESET, pid, jobs_list[0].cmd);
#endif
                while (jobs_list[0].pid > 0) {
                    pause();
                }
            } else {
                fprintf(stderr, ROJO "fork: %s\n"RESET, strerror(errno));
                exit(-2);
            }
        }
    }
    return EXIT_SUCCESS;
}
/**
 Parsea una línea de comandos.
 *args: array de punteros a char donde se almacenarán los argumentos.
 *line: línea de comandos a parsear.
 return: número de argumentos leídos.
 */
int parse_args(char **args, char *line) {
    const char *delimiters = " \t\n\r";
    char *token;
    unsigned int counter = 0;
    token = strtok(line,delimiters);
    // Mientras haya tokens, se añaden al array de argumentos
    while(token != NULL){
        if(token[0] == '#'){
#if DEBUGN1
            fprintf(stderr, GRIS "[parse_args()→ token %d: %s]\n" RESET, counter,token);
#endif
            token = NULL;
            args[counter] = token;
#if DEBUGN1
            fprintf(stderr, GRIS "[parse_args()→ token %d corregido: %s]\n" RESET, counter,token);
#endif
            return counter;
        }
        args[counter] = token;
#if DEBUGN1
        fprintf(stderr, GRIS "[parse_args()→ token %d: %s]\n" RESET, counter,token);
#endif
        counter++;
        token = strtok(NULL,delimiters);
    }
    // Se añade NULL al final del array de argumentos
    args[counter] = NULL;
#if DEBUGN1
    fprintf(stderr, GRIS "[parse_args()→ token %d: %s]\n" RESET, counter, token);
#endif
    return counter;
}
/**
 Checkea si el comando es interno.
 *args: array de punteros a char con los argumentos introducidos.
 return: 1 si es interno, 0 si no lo es.
 */
int check_internal(char **args) {
    int command = 0;
    // Si no hay argumentos, no se hace nada
    if (args == NULL || args[0] == NULL) {
        return -1;
    }
    else if (!strcmp(args[0], "cd")) {
        internal_cd(args);
        command = 1;
    }
    else if (!strcmp(args[0], "export")) {
        internal_export(args);
        command = 1;
    }
    else if (!strcmp(args[0], "source")) {
        internal_source(args);
        command = 1;
    }
    else if (!strcmp(args[0], "jobs")) {
        internal_jobs(args);
        command = 1;
    }
    else if (!strcmp(args[0], "fg")) {
        internal_fg(args);
        command = 1;
    }
    else if (!strcmp(args[0], "bg")) {
        internal_bg(args);
        command = 1;
    }
    else if (!strcmp(args[0], "exit")) {
        printf("Bye Bye\n");
        exit(0);
        return EXIT_SUCCESS;
    }
    return command;
}
/**
 Cambia de directorio.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien, -1 si hubo algún error.
 */
int internal_cd(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_cd()→ Esta función cambiará de directorio]\n" RESET);
#endif
    // Obtener el directorio HOME
    char *home_dir = getenv("HOME");    
    // Buffer para el directorio actual
    char current_dir[1024];           
    int result = -1;
    // Caso 1: Sin argumentos (ir al directorio HOME)
    if (args[1] == NULL) {
        if (home_dir == NULL) {
            fprintf(stderr, ROJO "Error: la variable HOME no está definida.\n" RESET);
            return EXIT_FAILURE;
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
        perror(ROJO "chdir() error");
        printf(RESET);
        return EXIT_FAILURE;
    }
    // Obtener el directorio actual después del cambio
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror(ROJO "chdir() error");
        printf(RESET);
        return EXIT_FAILURE;
    }
#if DEBUGN2
    fprintf(stderr, GRIS "[internal_cd()→ PWD: %s]\n" RESET, current_dir);
#endif
    return EXIT_SUCCESS;
}
/**
 Asigna valores a variables de entorno.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien, -1 si hubo algún error.
 */
int internal_export(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_export()→ Esta función asignará valores a variables cd de entorno]\n" RESET);
#endif
    // Verificar si el número de argumentos es adecuado (debe tener args[1])
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return EXIT_FAILURE;
    }
    // Buscar el primer '=' en args[1]
    char *equal_sign = strchr(args[1], '=');
    if (equal_sign == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return EXIT_FAILURE;
    }
    // Dividir en nombre y valor
    size_t name_len = equal_sign - args[1];
    char *name = (char *)malloc(name_len + 1);
    strncpy(name, args[1], name_len);
    name[name_len] = '\0';
    char *value = equal_sign + 1;
    // Verificar si el formato es correcto (debe haber un nombre y un valor)
    if (name == NULL || value == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return EXIT_FAILURE;
    }
    // Mostrar el valor inicial de la variable de entorno
    char *initial_value = getenv(name);
    if (initial_value != NULL) {
#if DEBUGN2
        printf(GRIS "[internal_export()→ nombre: %s]\n" RESET, name);
        printf(GRIS "[internal_export()→ valor: %s]\n" RESET, value);
        printf(GRIS "[internal_export()→ antiguo valor para %s: %s\n" RESET, name, initial_value);
#endif
    } else {
        fprintf(stderr, ROJO "Error: la variable %s no estaba definida.\n", name);
    }
    // Asignar el nuevo valor a la variable de entorno
    if (setenv(name, value, 1) != 0) {
        perror("setenv() error");
        return EXIT_FAILURE;
    }
    // Mostrar el nuevo valor de la variable de entorno
    char *new_value = getenv(name);
    if (new_value != NULL) {
#if DEBUGN2
        printf(GRIS "[internal_export()→ nuevo valor para %s: %s\n" RESET, name, new_value);
#endif
    } else {
        fprintf(stderr, ROJO "Error: la variable %s no se estableció correctamente.\n" RESET, name);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
/**
 Ejecuta un fichero de líneas de comandos.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien
 */
int internal_source(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_source()→ Esta función ejecutará un fichero de líneas de comandos]\n" RESET);
#endif
    // Comprobamos si el comando source tiene un argumento
    if (!args[1]) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: source <nombre fichero>\n" RESET);
        return EXIT_FAILURE;
    }
    // Abrimos el fichero en modo lectura
    char buffer[COMMAND_LINE_SIZE];
    FILE *file = fopen(args[1], "r");
    // Comprobamos si el fichero se ha abierto correctamente
    if (!file) {  
        fprintf(stderr, ROJO "fopen: %s\n" RESET, strerror(errno));
        return (EXIT_FAILURE);
    }
    // Leemos el fichero línea a línea
    while (fgets(buffer, COMMAND_LINE_SIZE, file) != NULL) {
        for (int i = 0; i < COMMAND_LINE_SIZE; i++) {
            if (buffer[i] == '\n') { 
                buffer[i] = '\0';
            }
        }
#if DEBUGN3        
        fprintf(stderr, GRIS "[internal_source() → LINE: %s]\n" RESET, buffer);
#endif       
        // Ejecutamos la línea
        fflush(file);
        execute_line(buffer); 
    }
    // Cerramos el fichero
    fclose(file); 
    return EXIT_FAILURE;
} 

void ctrlc(int signum) {
    // Reasignar signal() para capturar futuras señales SIGINT
    signal(SIGINT, ctrlc);

    // Imprimir un salto de línea para mantener un prompt limpio
    printf("\n");
    fflush(stdout);

    printf(GRIS"[ctrlc()→ Soy el proceso con PID %d (./nivel4), el proceso en foreground es %d (%s)]\n"RESET, getpid(),jobs_list[0].pid, jobs_list[0].cmd);

    // Verificar si hay un proceso en foreground
    if (jobs_list[0].pid > 0) {
        // Obtener el nombre del comando en ejecución en foreground
        char *foreground_cmd = jobs_list[0].cmd;
        
        // Verificar que el proceso en foreground NO sea el shell actual
        if (strncmp(foreground_cmd, mi_shell, strlen(mi_shell)) != 0) {
            // Enviar la señal SIGTERM al proceso en foreground
            if (kill(jobs_list[0].pid, SIGTERM) == 0) {
                printf(GRIS"[ctrlc()→ Señal %d enviada a %d (%s) por %d (./nivel4)]\n"RESET, SIGINT,jobs_list[0].pid, jobs_list[0].cmd, getpid());
            }
        } else {
            printf(GRIS"[ctrlc()→ Señal %d no enviada por %d (./nivel4) debido a que el proceso en foreground es el shell]\n"RESET, SIGINT,getpid());
        }
    } else {
        printf(GRIS"[ctrlc()→ Señal %d no enviada por %d (./nivel4) debido a que no hay proceso en foreground]\n"RESET, SIGINT,getpid());
    }

    fflush(stdout); // Asegurar la impresión inmediata del mensaje
}
void reaper(int signum) {
    int status;
    pid_t ended;

    // Reasignar la señal SIGCHLD al manejador
    signal(SIGCHLD, reaper);

    // Procesar todos los hijos que hayan terminado
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0) {

        // Mostrar información del hijo terminado
        if (WIFEXITED(status)) {
<<<<<<< HEAD
            printf(GRIS"[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d.\n]"RESET, ended, jobs_list[0].cmd, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf(GRIS"[reaper()→ Proceso hijo %d (%s) finalizado por señal %d.\n]"RESET, ended, jobs_list[0].cmd, WTERMSIG(status));
=======
            printf(GRIS"[reaper()→ Proceso hijo %d (%d) finalizado con exit code %d].\n"RESET, ended, jobs_list[0].cmd, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf(GRIS"[reaper()→ Proceso hijo %d (%d) finalizado por señal %d].\n"RESET, ended, jobs_list[0].cmd, WTERMSIG(status));
>>>>>>> 0a598b046644b9962fab713168fba4749cafce7d
        }

        // Si el hijo terminado es el proceso en primer plano
        if (ended == jobs_list[0].pid) {

            // Resetear el proceso en primer plano
            jobs_list[0].pid = 0;                        // PID a 0
            jobs_list[0].estado = 'F';                   // Estado 'Finished'
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE); // Resetear el comando
        }
    }

    // Manejar errores de waitpid
    if (ended == -1 && errno != ECHILD) {
        perror("Error en waitpid");
    }
}
/**
 Muestra los procesos que no están en foreground.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien
 */
int internal_jobs(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n" RESET);
#endif
    return EXIT_SUCCESS;
}
/**
 Trae los procesos más recientes al primer plano.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien
 */
int internal_fg(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_fg()→ Esta función traerá los procesos más recientes al primer plano]\n" RESET);
#endif
    return EXIT_SUCCESS;
}
/**
 Muestra los procesos parados o en segundo plano.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien
 */
int internal_bg(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_bg()→ Esta función mostrará los procesos parados o en segundo plano]\n" RESET);
#endif
    return EXIT_SUCCESS;
}
/**
 Imprime el prompt.
 */
void print_prompt(void) {
    user = getenv("USER");
    char path[200];
    getcwd(path, sizeof(path));

    printf(NEGRITA MAGENTA "%s" RESET, user);
    printf(NEGRITA ":" RESET);
    printf(NEGRITA CYAN "MINISHELL" RESET);
    printf(NEGRITA "%c " RESET, PROMPT);
    fflush(stdout);
}
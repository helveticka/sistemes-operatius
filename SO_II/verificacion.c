/**
 * @file verificacion.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */

 #include "verificacion.h"
 #define USE_READ_BLOCKS 1

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, RED"Sintaxis: verificacion <nombre_dispositivo> <directorio_simulacion>\n"RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED"Error al montar el dispositivo en ./verificacion\n"RESET);
        return FALLO;
    }

    char *dir_sim = argv[2];
    struct STAT stat;
    if (mi_stat(dir_sim, &stat) == FALLO) {
        fprintf(stderr, RED"Error al obtener stat de %s\n"RESET, dir_sim);
        bumount();
        return FALLO;
    }

    int numentradas = stat.tamEnBytesLog / sizeof(struct entrada);

#if DEBUGN13 || ENTREGA_3
    printf("dir_sim: %s\nnumentradas: %d NUMPROCESOS: %d\n", dir_sim, numentradas, NUMPROCESOS);
#endif

    if (numentradas != NUMPROCESOS) {
        fprintf(stderr, RED"ERROR: No coinciden las entradas con NUMPROCESOS\n"RESET);
        bumount();
        return FALLO;
    }

    char camino_informe[200];
    sprintf(camino_informe, "%sinforme.txt", dir_sim);
    if (mi_creat(camino_informe, 6) == FALLO) {
        fprintf(stderr, RED"Error creando %s\n"RESET, camino_informe);
        bumount();
        return FALLO;
    }

    struct entrada entradas[NUMPROCESOS];
    if (mi_read(dir_sim, (char *)entradas, 0, sizeof(entradas)) == FALLO) {
        fprintf(stderr, RED"Error leyendo directorio de simulacion\n"RESET);
        bumount();
        return FALLO;
    }
    int offset_write = 0;
    for (int i = 0; i < NUMPROCESOS; i++) {
        struct INFORMACION info = {0};
        int validadas = 0;

        entradas[i].nombre[sizeof(entradas[i].nombre) - 1] = '\0'; // seguridad
        sscanf(entradas[i].nombre, "proceso_%d", &info.pid);

        char fichero_path[200];
        sprintf(fichero_path, "%s%s/prueba.dat", dir_sim, entradas[i].nombre);

#if USE_READ_BLOCKS

    off_t offset = 0;

    struct REGISTRO buffer[BLOCKSIZE / sizeof(struct REGISTRO)];

    int leidos;


    while ((leidos = mi_read(fichero_path, (char *)buffer, offset, sizeof(buffer))) > 0) {

        int nregs = leidos / sizeof(struct REGISTRO);

        for (int j = 0; j < nregs; j++) {

            if (buffer[j].pid == info.pid) {

                if (validadas == 0) {

                    info.PrimeraEscritura = info.UltimaEscritura =

                    info.MenorPosicion = info.MayorPosicion = buffer[j];

                } else {

                    if (buffer[j].nEscritura < info.PrimeraEscritura.nEscritura)

                        info.PrimeraEscritura = buffer[j];

                    if (buffer[j].nEscritura > info.UltimaEscritura.nEscritura)

                        info.UltimaEscritura = buffer[j];

                    if (buffer[j].nRegistro < info.MenorPosicion.nRegistro)

                        info.MenorPosicion = buffer[j];

                    if (buffer[j].nRegistro > info.MayorPosicion.nRegistro)

                        info.MayorPosicion = buffer[j];

                }

                validadas++;

            }

        }

        offset += leidos;

        memset(buffer, 0, sizeof(buffer));

    }


#else

    off_t offset = 0;

    struct REGISTRO reg;


    while (mi_read(fichero_path, (char *)&reg, offset, sizeof(struct REGISTRO)) == sizeof(struct REGISTRO)) {

        if (reg.pid == info.pid) {

            if (validadas == 0) {

                info.PrimeraEscritura = info.UltimaEscritura =

                info.MenorPosicion = info.MayorPosicion = reg;

            } else {

                if (reg.nEscritura < info.PrimeraEscritura.nEscritura)

                    info.PrimeraEscritura = reg;

                if (reg.nEscritura > info.UltimaEscritura.nEscritura)

                    info.UltimaEscritura = reg;

                if (reg.nRegistro < info.MenorPosicion.nRegistro)

                    info.MenorPosicion = reg;

                if (reg.nRegistro > info.MayorPosicion.nRegistro)

                    info.MayorPosicion = reg;

            }

            validadas++;

        }

        offset += sizeof(struct REGISTRO);

    }

#endif

        info.nEscrituras = validadas;

#if DEBUGN13 || ENTREGA_3
        fprintf(stderr, GRAY"[%d) %u escrituras validadas en %s]\n" RESET, i + 1, info.nEscrituras, fichero_path);
#endif

        char linea[BLOCKSIZE];
        char *fecha_primera = malloc(200);
        struct tm *tm_primera = localtime(&info.PrimeraEscritura.fecha.tv_sec);
        strftime(fecha_primera, 64, "%Y-%m-%d %H:%M:%S", tm_primera);
        char fecha_primera_milis[100];
        sprintf(fecha_primera_milis, "%s.%06ld", fecha_primera, info.PrimeraEscritura.fecha.tv_usec);

        char *fecha_ultima = malloc(200);
        struct tm *tm_ultima = localtime(&info.UltimaEscritura.fecha.tv_sec);
        strftime(fecha_ultima, 64, "%Y-%m-%d %H:%M:%S", tm_ultima);
        char fecha_ultima_milis[100];
        sprintf(fecha_ultima_milis, "%s.%06ld", fecha_ultima, info.UltimaEscritura.fecha.tv_usec);

        char *fecha_menor = malloc(200);
        struct tm *tm_menor = localtime(&info.MenorPosicion.fecha.tv_sec);
        strftime(fecha_menor, 64, "%Y-%m-%d %H:%M:%S", tm_menor);
        char fecha_menor_milis[100];
        sprintf(fecha_menor_milis, "%s.%06ld", fecha_menor, info.MenorPosicion.fecha.tv_usec);

        char *fecha_mayor = malloc(200);
        struct tm *tm_mayor = localtime(&info.MayorPosicion.fecha.tv_sec);
        strftime(fecha_mayor, 64, "%Y-%m-%d %H:%M:%S", tm_mayor);
        char fecha_mayor_milis[100];
        sprintf(fecha_mayor_milis, "%s.%06ld", fecha_mayor, info.MayorPosicion.fecha.tv_usec);

        sprintf(linea, "\nPID: %d\n"
        "Numero de escrituras:\t%d\n"
        "Primera escritura:\t%d\t%d\t%s\n"
        "Ultima escritura:\t%d\t%d\t%s\n"
        "Menor posición:\t\t%d\t%d\t%s\n"
        "Mayor posición:\t\t%d\t%d\t%s\n",
        info.pid,
        info.nEscrituras,
        info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, fecha_primera_milis,
        info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, fecha_ultima_milis,
        info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, fecha_menor_milis,
        info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, fecha_mayor_milis);

        int bytes_escritos = mi_write(camino_informe, linea, offset_write, strlen(linea));
        if (bytes_escritos == FALLO) {
            fprintf(stderr, RED"Error escribiendo en el informe\n"RESET);
            bumount();
            return FALLO;
        }
        offset_write += bytes_escritos;
    }

    bumount();
    return EXITO;
}
/**
 * @file verificacion.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */

 #include "verificacion.h"

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

#if DEBUGN13
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

    for (int i = 0; i < NUMPROCESOS; i++) {
        struct INFORMACION info = {0};
        int validadas = 0;

        entradas[i].nombre[sizeof(entradas[i].nombre) - 1] = '\0'; // seguridad
        sscanf(entradas[i].nombre, "proceso_%d", &info.pid);

        char fichero_path[200];
        sprintf(fichero_path, "%s%s/prueba.dat", dir_sim, entradas[i].nombre);

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

        info.nEscrituras = validadas;

#if DEBUGN13
        printf("[%d) %u escrituras validadas en %s]\n", i + 1, info.nEscrituras, fichero_path);
#endif

        char linea[256];
        sprintf(linea, "PID: %d\nNumero de escrituras: %u\n", info.pid, info.nEscrituras);
        mi_write(camino_informe, linea, -1, strlen(linea));

        sprintf(linea, "Primera Escritura\t%d\t%d\t%s",
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                asctime(localtime(&info.PrimeraEscritura.fecha)));
        mi_write(camino_informe, linea, -1, strlen(linea));

        sprintf(linea, "Ultima Escritura\t%d\t%d\t%s",
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                asctime(localtime(&info.UltimaEscritura.fecha)));
        mi_write(camino_informe, linea, -1, strlen(linea));

        sprintf(linea, "Menor Posicion\t\t%d\t%d\t%s",
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                asctime(localtime(&info.MenorPosicion.fecha)));
        mi_write(camino_informe, linea, -1, strlen(linea));

        sprintf(linea, "Mayor Posicion\t\t%d\t%d\t%s",
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                asctime(localtime(&info.MayorPosicion.fecha)));
        mi_write(camino_informe, linea, -1, strlen(linea));
    }

    bumount();
    return EXITO;
}
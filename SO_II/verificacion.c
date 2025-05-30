#include "verificacion.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: verificacion <nombre_dispositivo> <directorio_simulación>\n"RESET);
        return FALLO;
    }
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED"Error al montar el dispositivo en ./verificacion\n"RESET);
        return FALLO;
    }

    char *dir_sim = argv[2];
    struct STAT stat;
    if (mi_stat(dir_sim, &stat) == FALLO) {
        fprintf(stderr, "Error al obtener stat de %s\n", dir_sim);
        bumount();
        return FALLO;
    }

    int numentradas = stat.tamEnBytesLog / sizeof(struct entrada);
    printf("dir_sim: %s\nnumentradas: %d NUMPROCESOS: %d\n", dir_sim, numentradas, NUMPROCESOS);

    if (numentradas != NUMPROCESOS) {
        fprintf(stderr, "ERROR: No coinciden las entradas con NUMPROCESOS\n");
        bumount();
        return FALLO;
    }

    // Crear informe.txt
    char camino_informe[200];
    sprintf(camino_informe, "%sinforme.txt", dir_sim);
    if (mi_creat(camino_informe, 6) == FALLO) {
        fprintf(stderr, "Error creando %s\n", camino_informe);
        bumount();
        return FALLO;
    }

    struct entrada entrada;
    struct INFORMACION info;
    struct REGISTRO buffer[BLOCKSIZE / sizeof(struct REGISTRO)];
    int total_validadas;
    int leidos;
    char fichero_path[200];
    char linea[256];

    for (int i = 0; i < numentradas; i++) {
        memset(&info, 0, sizeof(info));
        total_validadas = 0;

        if (mi_read(dir_sim, (char *)&entrada, i * sizeof(struct entrada), sizeof(struct entrada)) != sizeof(struct entrada)) {
            fprintf(stderr, "Error leyendo entrada %d\n", i);
            bumount();
            return FALLO;
        }

        sscanf(entrada.nombre, "proceso_%d", &info.pid);
        sprintf(fichero_path, "%s%s/prueba.dat", dir_sim, entrada.nombre);

        off_t offset = 0;
        while ((leidos = mi_read(fichero_path, (char *)buffer, offset, sizeof(buffer))) > 0) {
            int nregs = leidos / sizeof(struct REGISTRO);
            for (int j = 0; j < nregs; j++) {
                struct REGISTRO r = buffer[j];
                if (r.pid == info.pid) {
                    total_validadas++;
                    if (total_validadas == 1) {
                        info.PrimeraEscritura = info.UltimaEscritura = info.MenorPosicion = info.MayorPosicion = r;
                    } else {
                        if (r.nEscritura < info.PrimeraEscritura.nEscritura) info.PrimeraEscritura = r;
                        if (r.nEscritura > info.UltimaEscritura.nEscritura) info.UltimaEscritura = r;
                        if (r.nRegistro < info.MenorPosicion.nRegistro) info.MenorPosicion = r;
                        if (r.nRegistro > info.MayorPosicion.nRegistro) info.MayorPosicion = r;
                    }
                }
            }
            offset += leidos;
        }

        info.nEscrituras = total_validadas;

        printf("[%d) %u escrituras validadas en %s]\n", i + 1, info.nEscrituras, fichero_path);

        // Escribir resumen en informe.txt
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

        sprintf(linea, "Menor Posición\t\t%d\t%d\t%s",
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                asctime(localtime(&info.MenorPosicion.fecha)));
        mi_write(camino_informe, linea, -1, strlen(linea));

        sprintf(linea, "Mayor Posición\t\t%d\t%d\t%s",
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                asctime(localtime(&info.MayorPosicion.fecha)));
        mi_write(camino_informe, linea, -1, strlen(linea));
    }

    bumount();
    return 0;
}
#include "main.h"

static unsigned char **leer_ppm_pgm(const char *ruta, int *ancho, int *alto) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    char tipo[3];
    if (!fgets(tipo, sizeof(tipo), f)) { fclose(f); return NULL; }
    if (tipo[0] != 'P' || (tipo[1] != '2' && tipo[1] != '3')) {
        fclose(f);
        return NULL;
    }

    int c, done = 0;
    while (!done) {
        c = fgetc(f);
        if (c == '#') {
            while (c != '\n' && c != EOF) c = fgetc(f);
        } else {
            ungetc(c, f);
            done = 1;
        }
    }

    int maxv;
    if (fscanf(f, "%d %d %d", ancho, alto, &maxv) != 3) {
        fclose(f);
        return NULL;
    }

    unsigned char **mat = malloc((*alto) * sizeof(unsigned char *));
    if (!mat) { fclose(f); return NULL; }

    for (int y = 0; y < *alto; y++) {
        mat[y] = malloc((*ancho) * sizeof(unsigned char));
        if (!mat[y]) {
            for (int r = 0; r < y; r++) free(mat[r]);
            free(mat);
            fclose(f);
            return NULL;
        }
        for (int x = 0; x < *ancho; x++) {
            int r, g, b, v;
            if (tipo[1] == '2') {
                fscanf(f, "%d", &v);
            } else {
                fscanf(f, "%d %d %d", &r, &g, &b);
                v = (r + g + b) / 3;
            }
            if (v < 0) v = 0;
            if (v > 255) v = 255;
            mat[y][x] = (unsigned char)v;
        }
    }

    fclose(f);
    return mat;
}

int main() {
    char ruta[256];
    printf("Ingrese ruta del archivo .ppm/.pgm: ");
    if (!fgets(ruta, sizeof(ruta), stdin)) return 1;
    ruta[strcspn(ruta, "\n")] = 0;

    int ancho = 0, alto = 0;
    unsigned char **img = leer_ppm_pgm(ruta, &ancho, &alto);
    if (!img) {
        printf("No se pudo leer la imagen.\n");
        return 1;
    }

    char codigo[14];
    int dv;
    int encontrado = 0;

    for (int k = 30; k <= 70; k += 2) {
        int y = alto * k / 100;
        int r = decodificar_ean13_desde_grises(img[y], ancho, codigo, &dv);
        if (r == 0 || r == 2) {
            printf("Fila usada: %d\n", y);
            printf("Código EAN-13: %s\n", codigo);
            encontrado = 1;
            break;
        }
    }

    if (!encontrado)
        printf("No se pudo decodificar el código.\n");

    for (int i = 0; i < alto; i++) free(img[i]);
    free(img);

    return 0;
}
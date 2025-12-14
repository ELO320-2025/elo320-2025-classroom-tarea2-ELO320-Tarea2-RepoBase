#include "imagen.h"

unsigned char **leer_ppm_pgm(const char *ruta, int *ancho, int *alto) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    char tipo[3];
    if (!fgets(tipo, sizeof(tipo), f)) {
        fclose(f);
        return NULL;
    }

    if (tipo[0] != 'P' || (tipo[1] != '2' && tipo[1] != '3')) {
        fclose(f);
        return NULL;
    }

    int c;
    while ((c = fgetc(f)) == '#') {
        while (c != '\n' && c != EOF) c = fgetc(f);
    }
    ungetc(c, f);

    int maxv;
    if (fscanf(f, "%d %d %d", ancho, alto, &maxv) != 3) {
        fclose(f);
        return NULL;
    }

    unsigned char **img = malloc(*alto * sizeof(unsigned char *));
    if (!img) {
        fclose(f);
        return NULL;
    }

    for (int y = 0; y < *alto; y++) {
        img[y] = malloc(*ancho * sizeof(unsigned char));
        if (!img[y]) {
            for (int k = 0; k < y; k++) free(img[k]);
            free(img);
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
            img[y][x] = (unsigned char)v;
        }
    }

    fclose(f);
    return img;
}

void liberar_imagen(unsigned char **img, int alto) {
    for (int i = 0; i < alto; i++) free(img[i]);
    free(img);
}

unsigned char **copiar_imagen(unsigned char **img, int ancho, int alto) {
    unsigned char **out = malloc(alto * sizeof(unsigned char *));
    if (!out) return NULL;

    for (int y = 0; y < alto; y++) {
        out[y] = malloc(ancho * sizeof(unsigned char));
        if (!out[y]) {
            for (int k = 0; k < y; k++) free(out[k]);
            free(out);
            return NULL;
        }
        for (int x = 0; x < ancho; x++)
            out[y][x] = img[y][x];
    }
    return out;
}

unsigned char **img_binarizar(unsigned char **img, int ancho, int alto, int thres) {
    unsigned char **out = copiar_imagen(img, ancho, alto);
    if (!out) return NULL;

    for (int y = 0; y < alto; y++)
        for (int x = 0; x < ancho; x++)
            out[y][x] = (out[y][x] < thres) ? 0 : 255;

    return out;
}

unsigned char **img_contraste(unsigned char **img, int ancho, int alto, int newMin, int newMax) {
    unsigned char **out = copiar_imagen(img, ancho, alto);
    if (!out) return NULL;

    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
            int v = img[y][x];
            int nv = newMin + v * (newMax - newMin) / 255;
            if (nv < 0) nv = 0;
            if (nv > 255) nv = 255;
            out[y][x] = (unsigned char)nv;
        }
    }
    return out;
}

unsigned char **img_recortar(unsigned char **img, int *ancho, int *alto, int x, int y, int w, int h) {
    if (x < 0 || y < 0 || x + w > *ancho || y + h > *alto)
        return NULL;

    unsigned char **out = malloc(h * sizeof(unsigned char *));
    if (!out) return NULL;

    for (int i = 0; i < h; i++) {
        out[i] = malloc(w * sizeof(unsigned char));
        if (!out[i]) {
            for (int k = 0; k < i; k++) free(out[k]);
            free(out);
            return NULL;
        }
        for (int j = 0; j < w; j++)
            out[i][j] = img[y + i][x + j];
    }

    *ancho = w;
    *alto = h;
    return out;
}

unsigned char **img_rotar_90(unsigned char **img, int *ancho, int *alto) {
    int w = *ancho, h = *alto;
    unsigned char **out = malloc(w * sizeof(unsigned char *));
    if (!out) return NULL;

    for (int x = 0; x < w; x++) {
        out[x] = malloc(h * sizeof(unsigned char));
        if (!out[x]) {
            for (int k = 0; k < x; k++) free(out[k]);
            free(out);
            return NULL;
        }
        for (int y = 0; y < h; y++)
            out[x][y] = img[h - 1 - y][x];
    }

    *ancho = h;
    *alto = w;
    return out;
}

unsigned char **img_rotar_180(unsigned char **img, int *ancho, int *alto) {
    unsigned char **tmp = img_rotar_90(img, ancho, alto);
    unsigned char **out = img_rotar_90(tmp, ancho, alto);
    liberar_imagen(tmp, *alto);
    return out;
}

unsigned char **img_rotar_270(unsigned char **img, int *ancho, int *alto) {
    unsigned char **tmp = img_rotar_180(img, ancho, alto);
    unsigned char **out = img_rotar_90(tmp, ancho, alto);
    liberar_imagen(tmp, *alto);
    return out;
}

unsigned char **img_rotar(unsigned char **img, int *ancho, int *alto, int grados) {
    if (grados == 90) return img_rotar_90(img, ancho, alto);
    if (grados == 180) return img_rotar_180(img, ancho, alto);
    if (grados == 270) return img_rotar_270(img, ancho, alto);
    return NULL;
}

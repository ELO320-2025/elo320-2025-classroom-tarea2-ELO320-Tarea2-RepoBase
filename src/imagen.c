#include "imagen.h"

unsigned char **copiar_imagen(unsigned char **img, int ancho, int alto) {
    int y, x;
    unsigned char **out = malloc(alto * sizeof(unsigned char *));
    if (!out) return NULL;

    for (y = 0; y < alto; y++) {
        out[y] = malloc(ancho * sizeof(unsigned char));
        if (!out[y]) {
            for (int k = 0; k < y; k++) free(out[k]);
            free(out);
            return NULL;
        }
        for (x = 0; x < ancho; x++)
            out[y][x] = img[y][x];
    }
    return out;
}

void liberar_imagen(unsigned char **img, int alto) {
    for (int i = 0; i < alto; i++) free(img[i]);
    free(img);
}

unsigned char **binarizar_imagen(unsigned char **img, int ancho, int alto, int thres) {
    unsigned char **out = copiar_imagen(img, ancho, alto);
    if (!out) return NULL;

    for (int y = 0; y < alto; y++)
        for (int x = 0; x < ancho; x++)
            out[y][x] = (img[y][x] > thres) ? 255 : 0;

    return out;
}

unsigned char **ajustar_contraste(unsigned char **img, int ancho, int alto,
                                  int newMin, int newMax) {
    int Imin = 255;
    int Imax = 0;

    for (int y = 0; y < alto; y++)
        for (int x = 0; x < ancho; x++) {
            if (img[y][x] < Imin) Imin = img[y][x];
            if (img[y][x] > Imax) Imax = img[y][x];
        }

    if (Imax == Imin) return copiar_imagen(img, ancho, alto);

    unsigned char **out = copiar_imagen(img, ancho, alto);
    if (!out) return NULL;

    for (int y = 0; y < alto; y++)
        for (int x = 0; x < ancho; x++) {
            int v = img[y][x];
            int nv = (v - Imin) * (newMax - newMin) / (Imax - Imin) + newMin;
            if (nv < 0) nv = 0;
            if (nv > 255) nv = 255;
            out[y][x] = (unsigned char)nv;
        }

    return out;
}

unsigned char **recortar_imagen(unsigned char **img, int *ancho, int *alto, int x, int y, int w, int h) {
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

unsigned char **rotar_imagen_90(unsigned char **img, int *ancho, int *alto) {
    int w = *ancho;
    int h = *alto;

    unsigned char **out = malloc(w * sizeof(unsigned char *));
    if (!out) return NULL;

    for (int i = 0; i < w; i++) {
        out[i] = malloc(h * sizeof(unsigned char));
        if (!out[i]) {
            for (int k = 0; k < i; k++) free(out[k]);
            free(out);
            return NULL;
        }
        for (int j = 0; j < h; j++)
            out[i][j] = img[h - 1 - j][i];
    }

    *ancho = h;
    *alto = w;
    return out;
}

unsigned char **rotar_imagen_180(unsigned char **img, int *ancho, int *alto) {
    int w = *ancho;
    int h = *alto;

    unsigned char **out = copiar_imagen(img, w, h);
    if (!out) return NULL;

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            out[y][x] = img[h - 1 - y][w - 1 - x];

    return out;
}

unsigned char **rotar_imagen_270(unsigned char **img, int *ancho, int *alto) {
    int w = *ancho;
    int h = *alto;

    unsigned char **out = malloc(w * sizeof(unsigned char *));
    if (!out) return NULL;

    for (int i = 0; i < w; i++) {
        out[i] = malloc(h * sizeof(unsigned char));
        if (!out[i]) {
            for (int k = 0; k < i; k++) free(out[k]);
            free(out);
            return NULL;
        }
        for (int j = 0; j < h; j++)
            out[i][j] = img[j][w - 1 - i];
    }

    *ancho = h;
    *alto = w;
    return out;
}

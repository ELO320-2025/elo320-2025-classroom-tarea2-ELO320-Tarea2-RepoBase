#include "preproc.h"

Imagen *convertirAGris(Imagen *img) {
    if (!img || img->tipo != 3) return img;
    Imagen *g = crearImagen(img->ancho, img->alto, 2, img->maxVal);
    int i, j;
    for (i = 0; i < img->alto; i++) {
        for (j = 0; j < img->ancho; j++) {
            PixelRGB p = img->rgb[i][j];
            int v = (int)(0.3 * p.r + 0.59 * p.g + 0.11 * p.b);
            g->gris[i][j] = v;
        }
    }
    return g;
}

Imagen *binarizarImagen(Imagen *img, int umbral) {
    if (!img || img->tipo != 2) return img;
    Imagen *b = crearImagen(img->ancho, img->alto, 2, img->maxVal);
    int i, j;
    for (i = 0; i < img->alto; i++) {
        for (j = 0; j < img->ancho; j++) {
            int v = img->gris[i][j];
            b->gris[i][j] = v > umbral ? 1 : 0;
        }
    }
    return b;
}

Imagen *ajustarContraste(Imagen *img, int nuevoMin, int nuevoMax) {
    if (!img || img->tipo != 2) return img;
    int i, j;
    int min = img->gris[0][0];
    int max = img->gris[0][0];
    for (i = 0; i < img->alto; i++) {
        for (j = 0; j < img->ancho; j++) {
            int v = img->gris[i][j];
            if (v < min) min = v;
            if (v > max) max = v;
        }
    }
    Imagen *c = crearImagen(img->ancho, img->alto, 2, img->maxVal);
    for (i = 0; i < img->alto; i++) {
        for (j = 0; j < img->ancho; j++) {
            int v = img->gris[i][j];
            if (max == min) c->gris[i][j] = nuevoMin;
            else c->gris[i][j] = (v - min) * (nuevoMax - nuevoMin) / (max - min) + nuevoMin;
        }
    }
    return c;
}

Imagen *recortarImagen(Imagen *img, int x, int y, int w, int h) {
    if (!img) return NULL;
    if (x < 0 || y < 0 || x + w > img->ancho || y + h > img->alto) return NULL;

    Imagen *r = crearImagen(w, h, img->tipo, img->maxVal);
    int i, j;
    if (img->tipo == 2) {
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                r->gris[i][j] = img->gris[y + i][x + j];
            }
        }
    } else {
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                r->rgb[i][j] = img->rgb[y + i][x + j];
            }
        }
    }
    return r;
}

Imagen *rotarImagen90(Imagen *img) {
    if (!img) return NULL;
    Imagen *r = crearImagen(img->alto, img->ancho, img->tipo, img->maxVal);
    int i, j;
    if (img->tipo == 2) {
        for (i = 0; i < img->alto; i++) {
            for (j = 0; j < img->ancho; j++) {
                r->gris[j][img->alto - 1 - i] = img->gris[i][j];
            }
        }
    } else {
        for (i = 0; i < img->alto; i++) {
            for (j = 0; j < img->ancho; j++) {
                r->rgb[j][img->alto - 1 - i] = img->rgb[i][j];
            }
        }
    }
    return r;
}

Imagen *rotarImagen180(Imagen *img) {
    if (!img) return NULL;
    Imagen *r = crearImagen(img->ancho, img->alto, img->tipo, img->maxVal);
    int i, j;
    if (img->tipo == 2) {
        for (i = 0; i < img->alto; i++) {
            for (j = 0; j < img->ancho; j++) {
                r->gris[img->alto - 1 - i][img->ancho - 1 - j] = img->gris[i][j];
            }
        }
    } else {
        for (i = 0; i < img->alto; i++) {
            for (j = 0; j < img->ancho; j++) {
                r->rgb[img->alto - 1 - i][img->ancho - 1 - j] = img->rgb[i][j];
            }
        }
    }
    return r;
}

Imagen *rotarImagen270(Imagen *img) {
    if (!img) return NULL;
    Imagen *r = crearImagen(img->alto, img->ancho, img->tipo, img->maxVal);
    int i, j;
    if (img->tipo == 2) {
        for (i = 0; i < img->alto; i++) {
            for (j = 0; j < img->ancho; j++) {
                r->gris[img->ancho - 1 - j][i] = img->gris[i][j];
            }
        }
    } else {
        for (i = 0; i < img->alto; i++) {
            for (j = 0; j < img->ancho; j++) {
                r->rgb[img->ancho - 1 - j][i] = img->rgb[i][j];
            }
        }
    }
    return r;
}
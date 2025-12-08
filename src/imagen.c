#include "imagen.h"

Imagen *crearImagen(int ancho, int alto, int tipo, int maxVal) {
    Imagen *img = malloc(sizeof(Imagen));
    img->ancho = ancho;
    img->alto = alto;
    img->tipo = tipo;
    img->maxVal = maxVal;
    img->gris = NULL;
    img->rgb = NULL;
    if (tipo == 2) {
        img->gris = malloc(sizeof(int*) * alto);
        int i;
        for (i = 0; i < alto; i++) {
            img->gris[i] = malloc(sizeof(int) * ancho);
        }
    } else {
        img->rgb = malloc(sizeof(PixelRGB*) * alto);
        int i;
        for (i = 0; i < alto; i++) {
            img->rgb[i] = malloc(sizeof(PixelRGB) * ancho);
        }
    }
    return img;
}

void liberarImagen(Imagen *img) {
    if (!img) return;
    int i;
    if (img->gris) {
        for (i = 0; i < img->alto; i++) free(img->gris[i]);
        free(img->gris);
    }
    if (img->rgb) {
        for (i = 0; i < img->alto; i++) free(img->rgb[i]);
        free(img->rgb);
    }
    free(img);
}

Imagen *leerImagen(const char *ruta) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    char tipo[3];
    int ancho, alto, maxVal;
    if (fscanf(f, "%2s", tipo) != 1) {
        fclose(f);
        return NULL;
    }

    if (strcmp(tipo, "P2") == 0) {
        if (fscanf(f, "%d %d %d", &ancho, &alto, &maxVal) != 3) {
            fclose(f);
            return NULL;
        }
        Imagen *img = crearImagen(ancho, alto, 2, maxVal);
        int i, j;
        for (i = 0; i < alto; i++) {
            for (j = 0; j < ancho; j++) {
                fscanf(f, "%d", &img->gris[i][j]);
            }
        }
        fclose(f);
        return img;
    }

    if (strcmp(tipo, "P3") == 0) {
        if (fscanf(f, "%d %d %d", &ancho, &alto, &maxVal) != 3) {
            fclose(f);
            return NULL;
        }
        Imagen *img = crearImagen(ancho, alto, 3, maxVal);
        int i, j;
        for (i = 0; i < alto; i++) {
            for (j = 0; j < ancho; j++) {
                fscanf(f, "%d %d %d",
                       &img->rgb[i][j].r,
                       &img->rgb[i][j].g,
                       &img->rgb[i][j].b);
            }
        }
        fclose(f);
        return img;
    }

    fclose(f);
    return NULL;
}

int guardarImagenPGM(const char *ruta, Imagen *img) {
    if (!img || !img->gris) return 0;
    FILE *f = fopen(ruta, "w");
    if (!f) return 0;
    fprintf(f, "P2\n%d %d\n%d\n", img->ancho, img->alto, img->maxVal);
    int i, j;
    for (i = 0; i < img->alto; i++) {
        for (j = 0; j < img->ancho; j++) {
            fprintf(f, "%d ", img->gris[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return 1;
}

int guardarImagenPPM(const char *ruta, Imagen *img) {
    if (!img || !img->rgb) return 0;
    FILE *f = fopen(ruta, "w");
    if (!f) return 0;
    fprintf(f, "P3\n%d %d\n%d\n", img->ancho, img->alto, img->maxVal);
    int i, j;
    for (i = 0; i < img->alto; i++) {
        for (j = 0; j < img->ancho; j++) {
            PixelRGB p = img->rgb[i][j];
            fprintf(f, "%d %d %d ", p.r, p.g, p.b);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return 1;
}

Imagen *copiarImagen(Imagen *img) {
    if (!img) return NULL;
    Imagen *c = crearImagen(img->ancho, img->alto, img->tipo, img->maxVal);
    int i, j;
    if (img->tipo == 2) {
        for (i = 0; i < img->alto; i++) {
            for (j = 0; j < img->ancho; j++) {
                c->gris[i][j] = img->gris[i][j];
            }
        }
    } else {
        for (i = 0; i < img->alto; i++) {
            for (j = 0; j < img->ancho; j++) {
                c->rgb[i][j] = img->rgb[i][j];
            }
        }
    }
    return c;
}

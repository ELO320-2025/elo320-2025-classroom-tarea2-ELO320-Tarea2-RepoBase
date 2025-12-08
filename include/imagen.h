#ifndef IMAGEN_H
#define IMAGEN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    int r;
    int g;
    int b;
} PixelRGB;

typedef struct {
    int ancho;
    int alto;
    int maxVal;
    int tipo;
    int **gris;
    PixelRGB **rgb;
} Imagen;

Imagen *crearImagen(int ancho, int alto, int tipo, int maxVal);
void liberarImagen(Imagen *img);
Imagen *leerImagen(const char *ruta);
int guardarImagenPGM(const char *ruta, Imagen *img);
int guardarImagenPPM(const char *ruta, Imagen *img);
Imagen *copiarImagen(Imagen *img);

#endif

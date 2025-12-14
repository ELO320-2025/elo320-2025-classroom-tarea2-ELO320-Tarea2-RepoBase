#ifndef IMAGEN_H
#define IMAGEN_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned char **leer_ppm_pgm(const char *ruta, int *ancho, int *alto);
void liberar_imagen(unsigned char **img, int alto);
unsigned char **copiar_imagen(unsigned char **img, int ancho, int alto);

unsigned char **img_binarizar(unsigned char **img, int ancho, int alto, int thres);
unsigned char **img_contraste(unsigned char **img, int ancho, int alto, int newMin, int newMax);
unsigned char **img_recortar(unsigned char **img, int *ancho, int *alto, int x, int y, int w, int h);

unsigned char **img_rotar_90(unsigned char **img, int *ancho, int *alto);
unsigned char **img_rotar_180(unsigned char **img, int *ancho, int *alto);
unsigned char **img_rotar_270(unsigned char **img, int *ancho, int *alto);

unsigned char **img_rotar(unsigned char **img, int *ancho, int *alto, int grados);

#endif

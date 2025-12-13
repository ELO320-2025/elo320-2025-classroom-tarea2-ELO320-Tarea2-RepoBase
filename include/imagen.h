#ifndef IMAGEN_H
#define IMAGEN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

unsigned char **copiar_imagen(unsigned char **img, int ancho, int alto);

void liberar_imagen(unsigned char **img, int alto);

unsigned char **binarizar_imagen(unsigned char **img, int ancho, int alto, int thres);

unsigned char **ajustar_contraste(unsigned char **img, int ancho, int alto,  int newMin, int newMax);

unsigned char **recortar_imagen(unsigned char **img, int *ancho, int *alto, int x, int y, int w, int h);

unsigned char **rotar_imagen_90(unsigned char **img, int *ancho, int *alto);

unsigned char **rotar_imagen_180(unsigned char **img, int *ancho, int *alto);

unsigned char **rotar_imagen_270(unsigned char **img, int *ancho, int *alto);

#endif

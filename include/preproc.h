#ifndef PREPROC_H
#define PREPROC_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "imagen.h"

Imagen *convertirAGris(Imagen *img);
Imagen *binarizarImagen(Imagen *img, int umbral);
Imagen *ajustarContraste(Imagen *img, int nuevoMin, int nuevoMax);
Imagen *recortarImagen(Imagen *img, int x, int y, int w, int h);
Imagen *rotarImagen90(Imagen *img);
Imagen *rotarImagen180(Imagen *img);
Imagen *rotarImagen270(Imagen *img);

#endif
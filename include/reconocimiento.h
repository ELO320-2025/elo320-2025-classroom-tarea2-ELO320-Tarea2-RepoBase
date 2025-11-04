#ifndef RECONOCIMIENTO_H
#define RECONOCIMIENTO_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "imagen.h"

int raster_to_bits(const Imagen *row, int *bits, int max_bits);
int ean13_decode_from_bits(const int *bits, int nbits, char out_code[14], int *valido);
int ean13_verificar_digito(const char code[14]);

#endif


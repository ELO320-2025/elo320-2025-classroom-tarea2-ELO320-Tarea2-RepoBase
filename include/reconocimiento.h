#ifndef RECONOCIMIENTO_H
#define RECONOCIMIENTO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int d; 
    char p; 
    int e; 
}cand;

int decodificar_ean13_desde_grises(const unsigned char *fila, int ancho, char *codigo_out, int *digito_control_out);

#endif

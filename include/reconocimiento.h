#ifndef RECONOCIMIENTO_H
#define RECONOCIMIENTO_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    int *anchos;
    int largo;
} SecuenciaBarras;

typedef struct {
    int digitos[13];
    int valido;
} CodigoEAN13;

SecuenciaBarras obtenerSecuenciaBarras(int *fila, int largo);
SecuenciaBarras normalizarSecuencia(SecuenciaBarras s);
int detectarInicio(SecuenciaBarras s);
int detectarCentro(SecuenciaBarras s, int inicio);
CodigoEAN13 decodificarEAN13(SecuenciaBarras s, int inicio, int centro);
int validarControl(CodigoEAN13 c);

#endif

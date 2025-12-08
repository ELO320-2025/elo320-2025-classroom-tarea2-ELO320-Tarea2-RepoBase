#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "imagen.h"
#include "preproc.h"
#include "utilidades.h"
#include "reconocimiento.h"

typedef struct {
    int umbral;
    int nuevoMin;
    int nuevoMax;
    int rx;
    int ry;
    int rw;
    int rh;
} ParametrosPreproc;

typedef struct {
    char *ruta;
    int intentos;
    double tiempo;
    int resultado;
    CodigoEAN13 codigo;
} ResultadoImagen;

ParametrosPreproc leerParametros(const char *ruta);
Cola *leerColaImagenes(const char *ruta);
Hash *cargarInventario(const char *ruta);
void ejecutarModo1(Hash *inventario, Cola *cola, ParametrosPreproc p);
void ejecutarModo2(Hash *inventario, Cola *cola, ParametrosPreproc p);
void ejecutarModo3(Hash *inventario, Cola *cola, ParametrosPreproc p);

#endif

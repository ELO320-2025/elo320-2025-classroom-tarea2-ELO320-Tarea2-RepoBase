#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

int ean13_validar(const char *cod);

char *dupstr_local(const char *s);

typedef enum {
    RES_OK,
    RES_FALLO,
    RES_NO_INVENTARIADO,
    RES_ABORTADO
} EstadoResultado;

typedef struct {
    char *ruta;            
    EstadoResultado estado;       
    char ean[14];          
    char *producto;        
    int precio;            
    int intentos;          
    double tiempo_ms; 
    int orden;      
} ResultadoScan;

void imprimir_reporte_final(ResultadoScan *res, int n, int modo, int algoritmo);

#endif

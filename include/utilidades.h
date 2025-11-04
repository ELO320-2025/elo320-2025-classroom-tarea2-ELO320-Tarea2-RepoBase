#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    	int thres;
    	int newMin, newMax;
    	int recorte_linea[4];
    	int recorte_rect[4];
    	char **imagenes;
    	int n_imagenes;
} Config;

int cargar_config(const char *ruta, Config *cfg);
void liberar_config(Config *cfg);
void imprimir_config(const char *ruta, const Config *cfg);

void trim_inplace(char *s);
char *duplicar_cadena(const char *s);
int esta_vacia(const char *s);

#endif


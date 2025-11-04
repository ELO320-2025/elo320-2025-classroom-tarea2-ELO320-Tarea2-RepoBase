#ifndef PREPROC_H
#define PREPROC_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct Producto {
    	char *codigo;
    	char *nombre;
    	int precio;
	int orden;
    	struct Producto *sig;
} Producto;

typedef struct {
    	Producto **buckets;
    	size_t capacidad;
    	int elementos;
	int next_orden;
} TablaHash;

TablaHash *crear_TH(size_t capacidad);
void destruir_TH(TablaHash *th);
int insertar_TH(TablaHash *th, const char *codigo, const char *nombre, int precio);
Producto *buscar_TH(TablaHash *th, const char *codigo);
TablaHash *cargar_inventario(const char *ruta);
void imprimir_resumen_inventario(TablaHash *th, int max_items);

#endif


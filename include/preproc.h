#ifndef PREPROC_H
#define PREPROC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_INV_CAP 1021

typedef struct {
    char *nombre;
    int precio;
    char *codigo;
} ProductoInv;

typedef struct {
    ProductoInv **tabla;
    int capacidad;
} HashInv;

typedef struct NodoProd {
    char *nombre;
    int precio;
    char *codigo;
    struct NodoProd *sig;
} NodoProd;

typedef struct {
    int thres;
    int newMin;
    int newMax;
    int recX;
    int recY;
    int recW;
    int recH;
} Params;

typedef struct NImg {
    char *ruta;
    struct NImg *sig;
} NImg;

typedef struct {
    NImg *ini;
    NImg *fin;
    int total;
} ColaImagenes;

HashInv *crear_hash_inv();

void destruir_hash_inv(HashInv *h);

int insertar_inv(HashInv *h, const char *nombre, int precio, const char *codigo);

ProductoInv *buscar_inv(HashInv *h, const char *codigo);

HashInv *cargar_inventario(const char *ruta);

NodoProd *cargar_inventario_ordenado(const char *ruta, HashInv *h);

int contar_productos(NodoProd *lista);

void imprimir_inventario_ordenado(NodoProd *lista);

void init_params(Params *p);

void init_cola(ColaImagenes *c);

void push_cola(ColaImagenes *c, const char *ruta);

NImg *pop_cola(ColaImagenes *c);

int cargar_config(const char *ruta, Params *p, ColaImagenes *c);

#endif
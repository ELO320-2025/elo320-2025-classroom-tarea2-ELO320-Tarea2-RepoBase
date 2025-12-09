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

HashInv *crear_hash_inv();
void destruir_hash_inv(HashInv *h);
int insertar_inv(HashInv *h, const char *nombre, int precio, const char *codigo);
ProductoInv *buscar_inv(HashInv *h, const char *codigo);
HashInv *cargar_inventario(const char *ruta);

#endif

#endif
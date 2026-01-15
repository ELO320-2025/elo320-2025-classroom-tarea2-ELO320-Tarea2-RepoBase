#ifndef PREPROC_H
#define PREPROC_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "imagen.h"
#include "reconocimiento.h"

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

HashInv *crear_hash_inv();
void destruir_hash_inv(HashInv *h);
int insertar_inv(HashInv *h, const char *nombre, int precio, const char *codigo);
ProductoInv *buscar_inv(HashInv *h, const char *codigo);

NodoProd *cargar_inventario_ordenado_verificado(const char *ruta, HashInv *h, int *total, int *cambios_dv);

void imprimir_inventario_ordenado(NodoProd *lista);
void liberar_inventario_ordenado(NodoProd *lista);

typedef struct {
    int thres;
    int newMin;
    int newMax;
    int recX;
    int recY;
    int recW;
    int recH;
    int rot;
} Params;

void init_params(Params *p);
void imprimir_params(const Params *p);

typedef struct NImg {
    char *ruta;
    struct NImg *sig;
} NImg;

typedef struct {
    NImg *ini;
    NImg *fin;
    int total;
} ColaImagenes;

void init_cola(ColaImagenes *c);
void push_cola(ColaImagenes *c, const char *ruta);
NImg *pop_cola(ColaImagenes *c);

int cargar_config(const char *ruta, Params *p, ColaImagenes *c);

typedef enum {
    OP_BINARIZACION,
    OP_CONTRASTE,
    OP_RECORTE,
    OP_ROTACION
} TipoOperacion;

typedef struct NodoOp {
    TipoOperacion tipo;
    int valor;
    struct NodoOp *sig;
} NodoOp;

typedef struct {
    NodoOp *top;
    int size;
} PilaOps;

void init_pila(PilaOps *p);
int push_op(PilaOps *p, TipoOperacion tipo, int valor);
int pop_op(PilaOps *p);
void clear_pila(PilaOps *p);
unsigned char **aplicar_ops_desde_original(unsigned char **img_orig, int *ancho, int *alto, const Params *p, const PilaOps *ops);

#endif
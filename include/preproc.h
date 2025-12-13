#ifndef PREPROC_H
#define PREPROC_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

HashInv *crear_hash_inv();
void destruir_hash_inv(HashInv *h);
int insertar_inv(HashInv *h, const char *nombre, int precio, const char *codigo);
ProductoInv *buscar_inv(HashInv *h, const char *codigo);

int ean13_validar(const char *cod13);
int ean13_corregir_dv(char *cod13);

NodoProd *cargar_inventario_ordenado_verificado(const char *ruta, HashInv *h, int *total, int *cambios_dv);
int reescribir_inventario_archivo(const char *ruta, NodoProd *lista);
void liberar_inventario_ordenado(NodoProd *lista);
int contar_productos(NodoProd *lista);
void imprimir_inventario_ordenado(NodoProd *lista);

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

void init_params(Params *p);
void init_cola(ColaImagenes *c);
void push_cola(ColaImagenes *c, const char *ruta);
NImg *pop_cola(ColaImagenes *c);
int cargar_config(const char *ruta, Params *p, ColaImagenes *c);
void copiar_params(Params *dst, const Params *src);
void imprimir_params(const Params *p);
unsigned char **copiar_imagen(unsigned char **img, int ancho, int alto);
void liberar_imagen(unsigned char **img, int alto);
unsigned char **aplicar_binarizacion(unsigned char **img, int ancho, int alto, int thres);
unsigned char **aplicar_contraste(unsigned char **img, int ancho, int alto, int newMin, int newMax);
unsigned char **aplicar_recorte(unsigned char **img, int *ancho, int *alto, int x, int y, int w, int h);
unsigned char **aplicar_preprocesamiento(unsigned char **img_orig, int *ancho, int *alto, const Params *p);

typedef enum {
    OP_BINARIZACION,
    OP_CONTRASTE,
    OP_RECORTE
} TipoOperacion;

typedef struct NodoOp {
    TipoOperacion tipo;
    struct NodoOp *sig;
} NodoOp;

typedef struct {
    NodoOp *top;
    int size;
} PilaOps;

void init_pila(PilaOps *p);
int push_op(PilaOps *p, TipoOperacion tipo);
int pop_op(PilaOps *p);
void clear_pila(PilaOps *p);
unsigned char **aplicar_ops_desde_original(unsigned char **img_orig, int *ancho, int *alto, const Params *p, const PilaOps *ops);



#endif
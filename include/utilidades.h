#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct NodoS {
    void *dato;
    struct NodoS *sig;
} NodoS;

typedef struct {
    NodoS *ini;
    int largo;
} ListaSimple;

typedef struct NodoD {
    void *dato;
    struct NodoD *ant;
    struct NodoD *sig;
} NodoD;

typedef struct {
    NodoD *ini;
    NodoD *fin;
    int largo;
} ListaDoble;

typedef struct {
    ListaSimple *lista;
} Pila;

typedef struct {
    ListaDoble *lista;
} Cola;

typedef struct {
    void **datos;
    int capacidad;
    int ini;
    int fin;
    int lleno;
} BufferCircular;

typedef struct NodoABB {
    char *clave;
    void *valor;
    struct NodoABB *izq;
    struct NodoABB *der;
} NodoABB;

typedef struct {
    NodoABB *raiz;
} ABB;

typedef struct {
    char **claves;
    void **valores;
    int capacidad;
    int usados;
} Hash;

ListaSimple *crearListaSimple();
void insertarListaSimple(ListaSimple *l, void *dato);
void *obtenerListaSimple(ListaSimple *l, int pos);

ListaDoble *crearListaDoble();
void insertarFinalListaDoble(ListaDoble *l, void *dato);
void *extraerInicioListaDoble(ListaDoble *l);

Pila *crearPila();
void push(Pila *p, void *dato);
void *pop(Pila *p);
int pilaVacia(Pila *p);

Cola *crearCola();
void encolar(Cola *c, void *dato);
void *desencolar(Cola *c);
int colaVacia(Cola *c);

BufferCircular *crearBufferCircular(int capacidad);
int insertarBuffer(BufferCircular *b, void *dato);
void *extraerBuffer(BufferCircular *b);

ABB *crearABB();
void abbInsertar(ABB *a, char *clave, void *valor);
void *abbBuscar(ABB *a, char *clave);

Hash *crearHash(int capacidad);
void hashInsertar(Hash *h, const char *clave, void *valor);
void *hashBuscar(Hash *h, const char *clave);

#endif
#include "utilidades.h"

ListaSimple *crearListaSimple() {
    ListaSimple *l = malloc(sizeof(ListaSimple));
    l->ini = NULL;
    l->largo = 0;
    return l;
}

void insertarListaSimple(ListaSimple *l, void *dato) {
    NodoS *n = malloc(sizeof(NodoS));
    n->dato = dato;
    n->sig = l->ini;
    l->ini = n;
    l->largo++;
}

void *obtenerListaSimple(ListaSimple *l, int pos) {
    NodoS *a = l->ini;
    int i = 0;
    while (a && i < pos) {
        a = a->sig;
        i++;
    }
    if (!a) return NULL;
    return a->dato;
}

ListaDoble *crearListaDoble() {
    ListaDoble *l = malloc(sizeof(ListaDoble));
    l->ini = NULL;
    l->fin = NULL;
    l->largo = 0;
    return l;
}

void insertarFinalListaDoble(ListaDoble *l, void *dato) {
    NodoD *n = malloc(sizeof(NodoD));
    n->dato = dato;
    n->sig = NULL;
    n->ant = l->fin;
    if (l->fin) l->fin->sig = n;
    l->fin = n;
    if (!l->ini) l->ini = n;
    l->largo++;
}

void *extraerInicioListaDoble(ListaDoble *l) {
    if (!l->ini) return NULL;
    NodoD *n = l->ini;
    void *d = n->dato;
    l->ini = n->sig;
    if (l->ini) l->ini->ant = NULL;
    else l->fin = NULL;
    free(n);
    l->largo--;
    return d;
}

Pila *crearPila() {
    Pila *p = malloc(sizeof(Pila));
    p->lista = crearListaSimple();
    return p;
}

void push(Pila *p, void *dato) {
    insertarListaSimple(p->lista, dato);
}

void *pop(Pila *p) {
    if (!p->lista->ini) return NULL;
    NodoS *n = p->lista->ini;
    void *d = n->dato;
    p->lista->ini = n->sig;
    free(n);
    p->lista->largo--;
    return d;
}

int pilaVacia(Pila *p) {
    return p->lista->ini == NULL;
}

Cola *crearCola() {
    Cola *c = malloc(sizeof(Cola));
    c->lista = crearListaDoble();
    return c;
}

void encolar(Cola *c, void *dato) {
    insertarFinalListaDoble(c->lista, dato);
}

void *desencolar(Cola *c) {
    return extraerInicioListaDoble(c->lista);
}

int colaVacia(Cola *c) {
    return c->lista->ini == NULL;
}

BufferCircular *crearBufferCircular(int capacidad) {
    BufferCircular *b = malloc(sizeof(BufferCircular));
    b->datos = malloc(sizeof(void*) * capacidad);
    b->capacidad = capacidad;
    b->ini = 0;
    b->fin = 0;
    b->lleno = 0;
    return b;
}

int insertarBuffer(BufferCircular *b, void *dato) {
    if (b->lleno) return 0;
    b->datos[b->fin] = dato;
    b->fin = (b->fin + 1) % b->capacidad;
    if (b->fin == b->ini) b->lleno = 1;
    return 1;
}

void *extraerBuffer(BufferCircular *b) {
    if (!b->lleno && b->ini == b->fin) return NULL;
    void *d = b->datos[b->ini];
    b->ini = (b->ini + 1) % b->capacidad;
    b->lleno = 0;
    return d;
}

ABB *crearABB() {
    ABB *a = malloc(sizeof(ABB));
    a->raiz = NULL;
    return a;
}

static NodoABB *abbInsertarNodo(NodoABB *n, char *clave, void *valor) {
    if (!n) {
        n = malloc(sizeof(NodoABB));
        n->clave = strdup(clave);
        n->valor = valor;
        n->izq = NULL;
        n->der = NULL;
        return n;
    }
    int c = strcmp(clave, n->clave);
    if (c < 0) n->izq = abbInsertarNodo(n->izq, clave, valor);
    else if (c > 0) n->der = abbInsertarNodo(n->der, clave, valor);
    else n->valor = valor;
    return n;
}

void abbInsertar(ABB *a, char *clave, void *valor) {
    a->raiz = abbInsertarNodo(a->raiz, clave, valor);
}

static void *abbBuscarNodo(NodoABB *n, char *clave) {
    if (!n) return NULL;
    int c = strcmp(clave, n->clave);
    if (c == 0) return n->valor;
    if (c < 0) return abbBuscarNodo(n->izq, clave);
    return abbBuscarNodo(n->der, clave);
}

void *abbBuscar(ABB *a, char *clave) {
    return abbBuscarNodo(a->raiz, clave);
}

Hash *crearHash(int capacidad) {
    Hash *h = malloc(sizeof(Hash));
    h->capacidad = capacidad;
    h->usados = 0;
    h->claves = malloc(sizeof(char*) * capacidad);
    h->valores = malloc(sizeof(void*) * capacidad);
    int i;
    for (i = 0; i < capacidad; i++) {
        h->claves[i] = NULL;
        h->valores[i] = NULL;
    }
    return h;
}

static int hashFunc(const char *s, int cap) {
    int h = 0;
    int i;
    for (i = 0; s[i]; i++) h = (h * 31 + s[i]) % cap;
    return h;
}

void hashInsertar(Hash *h, const char *clave, void *valor) {
    if (h->usados == h->capacidad) return;
    int idx = hashFunc(clave, h->capacidad);
    while (h->claves[idx] != NULL) idx = (idx + 1) % h->capacidad;
    h->claves[idx] = strdup(clave);
    h->valores[idx] = valor;
    h->usados++;
}

void *hashBuscar(Hash *h, const char *clave) {
    int idx = hashFunc(clave, h->capacidad);
    int start = idx;
    while (h->claves[idx] != NULL) {
        if (strcmp(h->claves[idx], clave) == 0) return h->valores[idx];
        idx = (idx + 1) % h->capacidad;
        if (idx == start) break;
    }
    return NULL;
}

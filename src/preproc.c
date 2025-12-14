#include "preproc.h"

#define HASH_CAP 1021

static char *dupstr(const char *s) {
    char *r = malloc(strlen(s) + 1);
    if (r) strcpy(r, s);
    return r;
}

static unsigned long hash(const char *s) {
    unsigned long h = 5381;
    int c;
    while ((c = *s++)) h = ((h << 5) + h) + c;
    return h;
}

HashInv *crear_hash_inv() {
    HashInv *h = malloc(sizeof(HashInv));
    h->capacidad = HASH_CAP;
    h->tabla = calloc(h->capacidad, sizeof(ProductoInv *));
    return h;
}

void destruir_hash_inv(HashInv *h) {
    if (!h) return;
    for (int i = 0; i < h->capacidad; i++) {
        if (h->tabla[i]) {
            free(h->tabla[i]->nombre);
            free(h->tabla[i]->codigo);
            free(h->tabla[i]);
        }
    }
    free(h->tabla);
    free(h);
}

int insertar_inv(HashInv *h, const char *nombre, int precio, const char *codigo) {
    unsigned long k = hash(codigo) % h->capacidad;
    for (int i = 0; i < h->capacidad; i++) {
        int p = (k + i) % h->capacidad;
        if (!h->tabla[p]) {
            ProductoInv *pr = malloc(sizeof(ProductoInv));
            pr->nombre = dupstr(nombre);
            pr->codigo = dupstr(codigo);
            pr->precio = precio;
            h->tabla[p] = pr;
            return 1;
        }
    }
    return 0;
}

ProductoInv *buscar_inv(HashInv *h, const char *codigo) {
    unsigned long k = hash(codigo) % h->capacidad;
    for (int i = 0; i < h->capacidad; i++) {
        int p = (k + i) % h->capacidad;
        if (!h->tabla[p]) return NULL;
        if (strcmp(h->tabla[p]->codigo, codigo) == 0)
            return h->tabla[p];
    }
    return NULL;
}

void imprimir_inventario_ordenado(NodoProd *lista) {
    while (lista) {
        printf("  - %s | %d | %s\n",
               lista->nombre,
               lista->precio,
               lista->codigo);
        lista = lista->sig;
    }
}

NodoProd *cargar_inventario_ordenado_verificado(
    const char *ruta,
    HashInv *h,
    int *total,
    int *cambios
) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    NodoProd *ini = NULL, *fin = NULL;
    char linea[512];
    *total = 0;
    *cambios = 0;

    while (fgets(linea, sizeof(linea), f)) {
        char *nom = strtok(linea, "|");
        char *pre = strtok(NULL, "|");
        char *cod = strtok(NULL, "\n");
        if (!nom || !pre || !cod) continue;

        insertar_inv(h, nom, atoi(pre), cod);

        NodoProd *n = malloc(sizeof(NodoProd));
        n->nombre = dupstr(nom);
        n->precio = atoi(pre);
        n->codigo = dupstr(cod);
        n->sig = NULL;

        if (!ini) ini = fin = n;
        else { fin->sig = n; fin = n; }

        (*total)++;
    }

    fclose(f);
    return ini;
}

int reescribir_inventario_archivo(const char *ruta, NodoProd *l) {
    FILE *f = fopen(ruta, "w");
    if (!f) return 0;
    while (l) {
        fprintf(f, "%s|%d|%s\n", l->nombre, l->precio, l->codigo);
        l = l->sig;
    }
    fclose(f);
    return 1;
}

void liberar_inventario_ordenado(NodoProd *l) {
    while (l) {
        NodoProd *n = l->sig;
        free(l->nombre);
        free(l->codigo);
        free(l);
        l = n;
    }
}

void init_params(Params *p) {
    p->thres = 100;
    p->newMin = 0;
    p->newMax = 255;
    p->recX = 0;
    p->recY = 0;
    p->recW = 200;
    p->recH = 100;
    p->rot = 0;
}

void imprimir_params(const Params *p) {
    printf("  - Thres: %d\n", p->thres);
    printf("  - Contraste: [%d, %d]\n", p->newMin, p->newMax);
    printf("  - Recorte: [%d %d %d %d]\n", p->recX, p->recY, p->recW, p->recH);
}

void init_cola(ColaImagenes *c) {
    c->ini = c->fin = NULL;
    c->total = 0;
}

void push_cola(ColaImagenes *c, const char *ruta) {
    NImg *n = malloc(sizeof(NImg));
    n->ruta = dupstr(ruta);
    n->sig = NULL;
    if (!c->ini) c->ini = c->fin = n;
    else { c->fin->sig = n; c->fin = n; }
    c->total++;
}

NImg *pop_cola(ColaImagenes *c) {
    if (!c->ini) return NULL;
    NImg *n = c->ini;
    c->ini = n->sig;
    if (!c->ini) c->fin = NULL;
    c->total--;
    return n;
}

int cargar_config(const char *ruta, Params *p, ColaImagenes *c) {
    FILE *f = fopen(ruta, "r");
    if (!f) return 0;

    init_params(p);
    init_cola(c);

    char linea[256];
    int seccion = 0;

    while (fgets(linea, sizeof(linea), f)) {
        linea[strcspn(linea, "\n")] = 0;

        if (strlen(linea) == 0)
            continue;

        if (strcmp(linea, "Preprocesamiento params") == 0) {
            seccion = 1;
            continue;
        }

        if (strcmp(linea, "Imagenes") == 0) {
            seccion = 2;
            continue;
        }

        if (seccion == 1) {
            int a, b, x, y, w, h;

            if (sscanf(linea, "Thres %d", &a) == 1) {
                p->thres = a;
                continue;
            }

            if (sscanf(linea, "newMin %d newMax %d", &a, &b) == 2) {
                p->newMin = a;
                p->newMax = b;
                continue;
            }

            if (sscanf(linea, "Recorte %d %d %d %d", &x, &y, &w, &h) == 4) {
                p->recX = x;
                p->recY = y;
                p->recW = w;
                p->recH = h;
                continue;
            }

            continue;
        }

        if (seccion == 2) {
            push_cola(c, linea);
        }
    }

    fclose(f);
    return 1;
}

void init_pila(PilaOps *p) {
    p->top = NULL;
    p->size = 0;
}

int push_op(PilaOps *p, TipoOperacion t) {
    NodoOp *n = malloc(sizeof(NodoOp));
    n->tipo = t;
    n->sig = p->top;
    p->top = n;
    p->size++;
    return 1;
}

int pop_op(PilaOps *p) {
    if (!p->top) return 0;
    NodoOp *n = p->top;
    p->top = n->sig;
    free(n);
    p->size--;
    return 1;
}

void clear_pila(PilaOps *p) {
    while (p->top) pop_op(p);
}

static unsigned char **aplicar_una(
    unsigned char **img,
    int *ancho,
    int *alto,
    const Params *p,
    TipoOperacion t
) {
    unsigned char **out = img;

    if (t == OP_BINARIZACION)
        out = img_binarizar(img, *ancho, *alto, p->thres);
    else if (t == OP_CONTRASTE)
        out = img_contraste(img, *ancho, *alto, p->newMin, p->newMax);
    else if (t == OP_RECORTE)
        out = img_recortar(img, ancho, alto, p->recX, p->recY, p->recW, p->recH);
    else if (t == OP_ROTACION)
        out = img_rotar(img, ancho, alto, p->rot);

    liberar_imagen(img, *alto);
    return out;
}

unsigned char **aplicar_ops_desde_original(unsigned char **img_orig, int *ancho, int *alto, const Params *p, const PilaOps *ops) {
    unsigned char **img = copiar_imagen(img_orig, *ancho, *alto);
    if (!ops || ops->size == 0) return img;

    TipoOperacion orden[16];
    int i = ops->size - 1;
    for (NodoOp *n = ops->top; n; n = n->sig)
        orden[i--] = n->tipo;

    for (i = 0; i < ops->size; i++)
        img = aplicar_una(img, ancho, alto, p, orden[i]);

    return img;
}

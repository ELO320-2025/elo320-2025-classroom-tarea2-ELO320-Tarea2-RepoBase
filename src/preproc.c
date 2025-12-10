#include "preproc.h"

static unsigned long hash_str(const char *s) {
    unsigned long h = 5381;
    int c;
    while ((c = *s++)) h = ((h << 5) + h) + c;
    return h;
}

HashInv *crear_hash_inv() {
    HashInv *h = malloc(sizeof(HashInv));
    if (!h) return NULL;
    h->capacidad = HASH_INV_CAP;
    h->tabla = malloc(sizeof(ProductoInv*) * h->capacidad);
    if (!h->tabla) {
        free(h);
        return NULL;
    }
    int i;
    for (i = 0; i < h->capacidad; i++)
        h->tabla[i] = NULL;
    return h;
}

void destruir_hash_inv(HashInv *h) {
    if (!h) return;
    int i;
    for (i = 0; i < h->capacidad; i++) {
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
    unsigned long key = hash_str(codigo);
    int pos = key % h->capacidad;
    int i;

    for (i = 0; i < h->capacidad; i++) {
        int idx = (pos + i) % h->capacidad;
        if (!h->tabla[idx]) {
            ProductoInv *p = malloc(sizeof(ProductoInv));
            if (!p) return 0;
            p->nombre = strdup(nombre);
            p->codigo = strdup(codigo);
            p->precio = precio;
            h->tabla[idx] = p;
            return 1;
        }
    }
    return 0;
}

ProductoInv *buscar_inv(HashInv *h, const char *codigo) {
    unsigned long key = hash_str(codigo);
    int pos = key % h->capacidad;
    int i;

    for (i = 0; i < h->capacidad; i++) {
        int idx = (pos + i) % h->capacidad;
        if (!h->tabla[idx]) return NULL;
        if (strcmp(h->tabla[idx]->codigo, codigo) == 0)
            return h->tabla[idx];
    }
    return NULL;
}

HashInv *cargar_inventario(const char *ruta) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    HashInv *h = crear_hash_inv();
    if (!h) {
        fclose(f);
        return NULL;
    }

    char linea[512];
    while (fgets(linea, sizeof(linea), f)) {
        linea[strcspn(linea, "\n")] = 0;

        char *nom = strtok(linea, "|");
        char *precio_str = strtok(NULL, "|");
        char *codigo = strtok(NULL, "|");

        if (!nom || !precio_str || !codigo) continue;

        int precio = atoi(precio_str);
        insertar_inv(h, nom, precio, codigo);
    }

    fclose(f);
    return h;
}

NodoProd *cargar_inventario_ordenado(const char *ruta, HashInv *h) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    NodoProd *ini = NULL;
    NodoProd *fin = NULL;

    char linea[512];

    while (fgets(linea, sizeof(linea), f)) {
        linea[strcspn(linea, "\n")] = 0;

        char *nom = strtok(linea, "|");
        char *precio_str = strtok(NULL, "|");
        char *codigo = strtok(NULL, "|");

        if (!nom || !precio_str || !codigo) continue;

        int precio = atoi(precio_str);

        insertar_inv(h, nom, precio, codigo);

        NodoProd *n = malloc(sizeof(NodoProd));
        n->nombre = strdup(nom);
        n->precio = precio;
        n->codigo = strdup(codigo);
        n->sig = NULL;

        if (!ini) {
            ini = n;
            fin = n;
        } else {
            fin->sig = n;
            fin = n;
        }
    }

    fclose(f);
    return ini;
}

int contar_productos(NodoProd *lista) {
    int c = 0;
    while (lista) {
        c++;
        lista = lista->sig;
    }
    return c;
}

void imprimir_inventario_ordenado(NodoProd *lista) {
    while (lista) {
        printf("  - %s (%s)\n", lista->nombre, lista->codigo);
        lista = lista->sig;
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
}

void init_cola(ColaImagenes *c) {
    c->ini = NULL;
    c->fin = NULL;
    c->total = 0;
}

static char *dupstr(const char *s) {
    char *r = malloc(strlen(s) + 1);
    if (!r) return NULL;
    strcpy(r, s);
    return r;
}

void push_cola(ColaImagenes *c, const char *ruta) {
    NImg *n = malloc(sizeof(NImg));
    if (!n) return;
    n->ruta = dupstr(ruta);
    n->sig = NULL;
    if (!c->ini) {
        c->ini = n;
        c->fin = n;
    } else {
        c->fin->sig = n;
        c->fin = n;
    }
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
        if (strlen(linea) == 0) continue;

        if (strcmp(linea, "Preprocesamiento params") == 0) {
            seccion = 1;
            continue;
        }
        if (strcmp(linea, "Imagenes") == 0) {
            seccion = 2;
            continue;
        }

        if (seccion == 1) {
            if (strncmp(linea, "Thres", 5) == 0) {
                int t;
                if (sscanf(linea, "Thres %d", &t) == 1)
                    p->thres = t;
                continue;
            }

            if (strncmp(linea, "newMin", 6) == 0) {
                int a, b;
                if (sscanf(linea, "newMin %d newMax %d", &a, &b) == 2) {
                    p->newMin = a;
                    p->newMax = b;
                }
                continue;
            }

            if (strncmp(linea, "Recorte", 7) == 0) {
                int x, y, w, h;
                if (sscanf(linea, "Recorte %d %d %d %d", &x, &y, &w, &h) == 4) {
                    p->recX = x;
                    p->recY = y;
                    p->recW = w;
                    p->recH = h;
                }
                continue;
            }
        }

        if (seccion == 2) {
            push_cola(c, linea);
        }
    }

    fclose(f);
    return 1;
}

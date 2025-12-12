#include "preproc.h"

static char *dupstr_local(const char *s) {
    char *r = malloc(strlen(s) + 1);
    if (!r) return NULL;
    strcpy(r, s);
    return r;
}

static unsigned long hash_str(const char *s) {
    unsigned long h = 5381;
    int c;
    while ((c = *s++)) h = ((h << 5) + h) + (unsigned long)c;
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
    for (i = 0; i < h->capacidad; i++) h->tabla[i] = NULL;
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
    if (!h || !nombre || !codigo) return 0;

    unsigned long key = hash_str(codigo);
    int pos = (int)(key % (unsigned long)h->capacidad);
    int i;

    for (i = 0; i < h->capacidad; i++) {
        int idx = (pos + i) % h->capacidad;

        if (!h->tabla[idx]) {
            ProductoInv *p = malloc(sizeof(ProductoInv));
            if (!p) return 0;
            p->nombre = dupstr_local(nombre);
            p->codigo = dupstr_local(codigo);
            p->precio = precio;
            if (!p->nombre || !p->codigo) {
                free(p->nombre);
                free(p->codigo);
                free(p);
                return 0;
            }
            h->tabla[idx] = p;
            return 1;
        }

        if (strcmp(h->tabla[idx]->codigo, codigo) == 0) {
            free(h->tabla[idx]->nombre);
            h->tabla[idx]->nombre = dupstr_local(nombre);
            h->tabla[idx]->precio = precio;
            return 1;
        }
    }

    return 0;
}

ProductoInv *buscar_inv(HashInv *h, const char *codigo) {
    if (!h || !codigo) return NULL;

    unsigned long key = hash_str(codigo);
    int pos = (int)(key % (unsigned long)h->capacidad);
    int i;

    for (i = 0; i < h->capacidad; i++) {
        int idx = (pos + i) % h->capacidad;
        if (!h->tabla[idx]) return NULL;
        if (strcmp(h->tabla[idx]->codigo, codigo) == 0) return h->tabla[idx];
    }

    return NULL;
}

static int ean13_dv_de_12(const char *cod13) {
    int s1 = 0, s2 = 0;
    int i;
    for (i = 0; i < 12; i++) {
        int v = cod13[i] - '0';
        if (i % 2 == 0) s1 += v;
        else s2 += v;
    }
    {
        int t = s1 + 3 * s2;
        int r = t % 10;
        return (r == 0) ? 0 : 10 - r;
    }
}

int ean13_validar(const char *cod13) {
    if (!cod13) return 0;
    if ((int)strlen(cod13) != 13) return 0;

    int i;
    for (i = 0; i < 13; i++)
        if (cod13[i] < '0' || cod13[i] > '9') return 0;

    {
        int dv = ean13_dv_de_12(cod13);
        return dv == (cod13[12] - '0');
    }
}

int ean13_corregir_dv(char *cod13) {
    if (!cod13) return 0;
    if ((int)strlen(cod13) != 13) return 0;

    int i;
    for (i = 0; i < 13; i++)
        if (cod13[i] < '0' || cod13[i] > '9') return 0;

    {
        int old = cod13[12] - '0';
        int dv = ean13_dv_de_12(cod13);
        cod13[12] = (char)('0' + dv);
        return dv != old;
    }
}

NodoProd *cargar_inventario_ordenado_verificado(const char *ruta, HashInv *h, int *total, int *cambios_dv) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    NodoProd *ini = NULL;
    NodoProd *fin = NULL;

    int cnt = 0;
    int cambios = 0;

    char linea[512];

    while (fgets(linea, sizeof(linea), f)) {
        linea[strcspn(linea, "\n")] = 0;
        if (strlen(linea) == 0) continue;

        char *nom = strtok(linea, "|");
        char *precio_str = strtok(NULL, "|");
        char *codigo = strtok(NULL, "|");

        if (!nom || !precio_str || !codigo) continue;

        int precio = atoi(precio_str);

        char codbuf[64];
        strncpy(codbuf, codigo, sizeof(codbuf));
        codbuf[sizeof(codbuf) - 1] = 0;

        if ((int)strlen(codbuf) == 13) {
            if (!ean13_validar(codbuf)) {
                if (ean13_corregir_dv(codbuf)) cambios++;
            }
        }

        insertar_inv(h, nom, precio, codbuf);

        NodoProd *n = malloc(sizeof(NodoProd));
        if (!n) continue;

        n->nombre = dupstr_local(nom);
        n->precio = precio;
        n->codigo = dupstr_local(codbuf);
        n->sig = NULL;

        if (!n->nombre || !n->codigo) {
            free(n->nombre);
            free(n->codigo);
            free(n);
            continue;
        }

        if (!ini) {
            ini = n;
            fin = n;
        } else {
            fin->sig = n;
            fin = n;
        }

        cnt++;
    }

    fclose(f);

    if (total) *total = cnt;
    if (cambios_dv) *cambios_dv = cambios;

    return ini;
}

int reescribir_inventario_archivo(const char *ruta, NodoProd *lista) {
    FILE *f = fopen(ruta, "w");
    if (!f) return 0;

    while (lista) {
        fprintf(f, "%s|%d|%s\n", lista->nombre, lista->precio, lista->codigo);
        lista = lista->sig;
    }

    fclose(f);
    return 1;
}

void liberar_inventario_ordenado(NodoProd *lista) {
    while (lista) {
        NodoProd *nx = lista->sig;
        free(lista->nombre);
        free(lista->codigo);
        free(lista);
        lista = nx;
    }
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

void push_cola(ColaImagenes *c, const char *ruta) {
    NImg *n = malloc(sizeof(NImg));
    if (!n) return;
    n->ruta = dupstr_local(ruta);
    n->sig = NULL;

    if (!n->ruta) {
        free(n);
        return;
    }

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
                if (sscanf(linea, "Thres %d", &t) == 1) p->thres = t;
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

void copiar_params(Params *dst, const Params *src) {
    *dst = *src;
}

void imprimir_params(const Params *p) {
    printf("  - Thres: %d\n", p->thres);
    printf("  - Contraste: [%d, %d]\n", p->newMin, p->newMax);
    printf("  - Recorte: [%d, %d, %d, %d]\n",
           p->recX, p->recY, p->recW, p->recH);
}

unsigned char **copiar_imagen(unsigned char **img, int ancho, int alto) {
    int y, x;
    unsigned char **out = malloc(alto * sizeof(unsigned char *));
    if (!out) return NULL;

    for (y = 0; y < alto; y++) {
        out[y] = malloc(ancho * sizeof(unsigned char));
        if (!out[y]) {
            int k;
            for (k = 0; k < y; k++) free(out[k]);
            free(out);
            return NULL;
        }
        for (x = 0; x < ancho; x++)
            out[y][x] = img[y][x];
    }
    return out;
}

void liberar_imagen(unsigned char **img, int alto) {
    int i;
    for (i = 0; i < alto; i++) free(img[i]);
    free(img);
}

unsigned char **aplicar_binarizacion(unsigned char **img, int ancho, int alto, int thres) {
    int y, x;
    unsigned char **out = copiar_imagen(img, ancho, alto);
    if (!out) return NULL;

    for (y = 0; y < alto; y++)
        for (x = 0; x < ancho; x++)
            out[y][x] = (out[y][x] < thres) ? 0 : 255;

    return out;
}

unsigned char **aplicar_contraste(unsigned char **img, int ancho, int alto,
                                  int newMin, int newMax) {
    int y, x;
    unsigned char **out = copiar_imagen(img, ancho, alto);
    if (!out) return NULL;

    for (y = 0; y < alto; y++) {
        for (x = 0; x < ancho; x++) {
            int v = img[y][x];
            int nv = newMin + v * (newMax - newMin) / 255;
            if (nv < 0) nv = 0;
            if (nv > 255) nv = 255;
            out[y][x] = (unsigned char)nv;
        }
    }
    return out;
}

unsigned char **aplicar_recorte(unsigned char **img,
                                 int *ancho, int *alto,
                                 int x, int y, int w, int h) {
    int i, j;

    if (x < 0 || y < 0 || x + w > *ancho || y + h > *alto)
        return NULL;

    unsigned char **out = malloc(h * sizeof(unsigned char *));
    if (!out) return NULL;

    for (i = 0; i < h; i++) {
        out[i] = malloc(w * sizeof(unsigned char));
        if (!out[i]) {
            int k;
            for (k = 0; k < i; k++) free(out[k]);
            free(out);
            return NULL;
        }
        for (j = 0; j < w; j++)
            out[i][j] = img[y + i][x + j];
    }

    *ancho = w;
    *alto = h;
    return out;
}

/**

unsigned char **aplicar_preprocesamiento(unsigned char **img_orig, int *ancho, int *alto, const Params *p) {
    unsigned char **tmp1 = aplicar_binarizacion(img_orig, *ancho, *alto, p->thres);
    if (!tmp1) return NULL;

    unsigned char **tmp2 = aplicar_contraste(tmp1, *ancho, *alto, p->newMin, p->newMax);
    liberar_imagen(tmp1, *alto);
    if (!tmp2) return NULL;

    unsigned char **out = aplicar_recorte(tmp2, ancho, alto, p->recX, p->recY, p->recW, p->recH);
    liberar_imagen(tmp2, *alto);

    return out;
}

**/
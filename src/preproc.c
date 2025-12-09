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

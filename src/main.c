#include "main.h"

static unsigned char **leer_ppm_pgm(const char *ruta, int *ancho, int *alto) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    char tipo[3];
    if (!fgets(tipo, sizeof(tipo), f)) {
        fclose(f);
        return NULL;
    }

    if (tipo[0] != 'P' || (tipo[1] != '2' && tipo[1] != '3')) {
        fclose(f);
        return NULL;
    }

    int c, done = 0;
    while (!done) {
        c = fgetc(f);
        if (c == '#') {
            while (c != '\n' && c != EOF) c = fgetc(f);
        } else {
            ungetc(c, f);
            done = 1;
        }
    }

    int maxv;
    if (fscanf(f, "%d %d %d", ancho, alto, &maxv) != 3) {
        fclose(f);
        return NULL;
    }

    unsigned char **mat = malloc((*alto) * sizeof(unsigned char *));
    if (!mat) {
        fclose(f);
        return NULL;
    }

    int y, x;
    for (y = 0; y < *alto; y++) {
        mat[y] = malloc((*ancho) * sizeof(unsigned char));
        if (!mat[y]) {
            int k;
            for (k = 0; k < y; k++) free(mat[k]);
            free(mat);
            fclose(f);
            return NULL;
        }
        for (x = 0; x < *ancho; x++) {
            int r, g, b, v;
            if (tipo[1] == '2') {
                fscanf(f, "%d", &v);
            } else {
                fscanf(f, "%d %d %d", &r, &g, &b);
                v = (r + g + b) / 3;
            }
            if (v < 0) v = 0;
            if (v > 255) v = 255;
            mat[y][x] = (unsigned char)v;
        }
    }

    fclose(f);
    return mat;
}

static void liberar_imagen(unsigned char **img, int alto) {
    int i;
    for (i = 0; i < alto; i++) free(img[i]);
    free(img);
}

static void modo_manual() {
    char ruta[256];
    int ancho = 0, alto = 0;
    char codigo[14];
    int dv;

    printf("Ingrese nombre de la imagen: ");
    fflush(stdout);

    if (!fgets(ruta, sizeof(ruta), stdin)) return;
    ruta[strcspn(ruta, "\n")] = 0;

    unsigned char **img = leer_ppm_pgm(ruta, &ancho, &alto);
    if (!img) {
        printf("No se pudo leer la imagen.\n");
        return;
    }

    int encontrado = 0;
    int k;
    for (k = 30; k <= 70; k += 2) {
        int y = alto * k / 100;
        int r = decodificar_ean13_desde_grises(img[y], ancho, codigo, &dv);
        if (r == 0 || r == 2) {
            printf("Fila usada: %d\n", y);
            printf("Código EAN-13: %s\n", codigo);
            encontrado = 1;
            break;
        }
    }

    if (!encontrado)
        printf("No se pudo decodificar el código.\n");

    liberar_imagen(img, alto);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: ./EDAcodebar <modo> <inventario> <config>\n");
        return 1;
    }

    int modo = atoi(argv[1]);
    const char *ruta_inv = argv[2];
    const char *ruta_cfg = argv[3];

    HashInv *inv = crear_hash_inv();
    NodoProd *listaInv = cargar_inventario_ordenado(ruta_inv, inv);
    if (!listaInv) {
        printf("No se pudo cargar inventario.\n");
        destruir_hash_inv(inv);
        return 1;
    }

    Params params;
    ColaImagenes cola;
    int ok = cargar_config(ruta_cfg, &params, &cola);
    if (!ok) {
        printf("No se pudo cargar configuración.\n");
        destruir_hash_inv(inv);
        return 1;
    }

    int totalProductos = contar_productos(listaInv);

    printf("Cargando inventario desde '%s'... (%d productos)\n", ruta_inv, totalProductos);
    printf("Cargando cola de escaneo desde '%s'... (%d imágenes)\n", ruta_cfg, cola.total);
    printf("\n---\n");

    printf("- Inventario Cargado: %d productos\n", totalProductos);
    imprimir_inventario_ordenado(listaInv);

    printf("\n- Imágenes en Cola (%d):\n", cola.total);
    NImg *p = cola.ini;
    while (p) {
        printf("  - %s\n", p->ruta);
        p = p->sig;
    }

    printf("\n- Parámetros de Preprocesamiento Inicial:\n");
    printf("  - Thres: %d\n", params.thres);
    printf("  - Contraste: [%d, %d]\n", params.newMin, params.newMax);
    printf("  - Recorte: [%d, %d, %d, %d]\n", params.recX, params.recY, params.recW, params.recH);
    printf("---\n");

    if (modo == 1) {
        modo_manual();
    } else if (modo == 2) {
        printf("Modo automático no implementado aún.\n");
    } else if (modo == 3) {
        printf("Modo semi-automático no implementado aún.\n");
    } else {
        printf("Modo inválido.\n");
    }

    destruir_hash_inv(inv);
    return 0;
}
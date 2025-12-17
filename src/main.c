#include "main.h"

static int extraer_ean_de_ruta(const char *ruta, char *out) {
    int len = strlen(ruta);
    int i, j;

    for (i = 0; i + 12 < len; i++) {
        int ok = 1;
        for (j = 0; j < 13; j++) {
            if (ruta[i + j] < '0' || ruta[i + j] > '9') {
                ok = 0;
                break;
            }
        }
        if (ok) {
            memcpy(out, ruta + i, 13);
            out[13] = 0;
            return 1;
        }
    }
    return 0;
}

static void buscar_codigo_en_imagen(
    unsigned char **img,
    int ancho,
    int alto,
    const char *ruta_img,
    HashInv *inv
) {
    char codigo[14];
    int dv_img = 0;
    int encontrado = 0;
    int r = 1;

    for (int k = 30; k <= 70; k += 2) {
        int y = alto * k / 100;
        r = decodificar_ean13_desde_grises(img[y], ancho, codigo, &dv_img);
        if (r == 0 || r == 2) {
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("ERROR: No se pudo decodificar el código EAN-13. Patrones de barras no reconocidos.\n");
        return;
    }

    printf("Código leído: %s\n", codigo);

    if (r == 2)
        printf("Advertencia: dígito verificador incorrecto en código leído\n");

    char ean_arch[14];
    if (extraer_ean_de_ruta(ruta_img, ean_arch)) {
        if (!ean13_validar(ean_arch))
            printf("Advertencia: DV incorrecto en nombre del archivo\n");
        else if (strcmp(ean_arch, codigo) != 0)
            printf("Advertencia: código leído no coincide con nombre del archivo\n");
    }

    ProductoInv *p = buscar_inv(inv, codigo);
    if (p) {
        printf("Producto encontrado\n");
        printf("Nombre : %s\n", p->nombre);
        printf("Precio : %d\n", p->precio);
    } else {
        printf("Código válido pero no inventariado\n");
    }
}

static void imprimir_procesamiento(const PilaOps *ops, const Params *p) {
    TipoOperacion orden[16];
    int n = ops->size;
    int i = n - 1;
    NodoOp *cur = ops->top;

    while (cur && i >= 0) {
        orden[i--] = cur->tipo;
        cur = cur->sig;
    }

    int idx = 1;
    printf("Procesamiento actual:\n");

    for (i = 0; i < n; i++) {
        if (orden[i] == OP_BINARIZACION)
            printf("%d. Binarización (T=%d)\n", idx++, p->thres);
        else if (orden[i] == OP_CONTRASTE)
            printf("%d. Contraste [%d, %d]\n", idx++, p->newMin, p->newMax);
        else if (orden[i] == OP_RECORTE)
            printf("%d. Recorte [%d, %d, %d, %d]\n",
                   idx++, p->recX, p->recY, p->recW, p->recH);
        else if (orden[i] == OP_ROTACION)
            printf("%d. Rotación %d°\n", idx++, p->rot);
    }
}

static void modo_manual(HashInv *inv, ColaImagenes *cola, Params *params_base) {
    while (cola->total > 0) {
        NImg *nimg = pop_cola(cola);
        if (!nimg) return;

        int ancho = 0, alto = 0;
        unsigned char **img_orig = leer_ppm_pgm(nimg->ruta, &ancho, &alto);
        if (!img_orig) {
            printf("No se pudo leer imagen %s\n", nimg->ruta);
            free(nimg->ruta);
            free(nimg);
            continue;
        }

        Params params_actual = *params_base;

        PilaOps ops;
        init_pila(&ops);
        push_op(&ops, OP_BINARIZACION);
        push_op(&ops, OP_CONTRASTE);
        push_op(&ops, OP_RECORTE);

        int ancho_p = ancho;
        int alto_p = alto;
        unsigned char **img_proc = aplicar_ops_desde_original(
            img_orig, &ancho_p, &alto_p, &params_actual, &ops
        );

        int terminado = 0;

        while (!terminado) {
            printf("\nImagen: %s\n", nimg->ruta);
            imprimir_procesamiento(&ops, &params_actual);

            printf("\nOpciones:\n");
            printf("1. Undo último paso\n");
            printf("2. Agregar procesamiento\n");
            printf("3. Buscar código de barras\n");
            printf("4. Abortar imagen\n");
            printf("Seleccione opción: ");

            char buf[32];
            if (!fgets(buf, sizeof(buf), stdin)) break;
            int op = atoi(buf);

            if (op == 1) {
                if (!pop_op(&ops)) {
                    printf("No hay más pasos para deshacer\n");
                } else {
                    liberar_imagen(img_proc, alto_p);
                    ancho_p = ancho;
                    alto_p = alto;
                    img_proc = aplicar_ops_desde_original(
                        img_orig, &ancho_p, &alto_p, &params_actual, &ops
                    );
                }
            }

            else if (op == 2) {
                printf("Funcionalidad de agregar procesamiento pendiente\n");
            }

            else if (op == 3) {
                buscar_codigo_en_imagen(img_proc, ancho_p, alto_p, nimg->ruta, inv);
                terminado = 1;
            }

            else if (op == 4) {
                printf("Imagen abortada\n");
                terminado = 1;
            }

            else {
                printf("Opción inválida\n");
            }
        }

        liberar_imagen(img_proc, alto_p);
        liberar_imagen(img_orig, alto);
        clear_pila(&ops);
        free(nimg->ruta);
        free(nimg);
    }
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
    int total = 0, cambios = 0;

    NodoProd *listaInv = cargar_inventario_ordenado_verificado(
        ruta_inv, inv, &total, &cambios
    );

    if (!listaInv) {
        printf("No se pudo cargar inventario\n");
        destruir_hash_inv(inv);
        return 1;
    }

    if (cambios > 0)
        reescribir_inventario_archivo(ruta_inv, listaInv);

    Params params_base;
    ColaImagenes cola;

    if (!cargar_config(ruta_cfg, &params_base, &cola)) {
        printf("No se pudo cargar configuración\n");
        liberar_inventario_ordenado(listaInv);
        destruir_hash_inv(inv);
        return 1;
    }

    printf("- Parámetros de Preprocesamiento Inicial:\n");
    imprimir_params(&params_base);

    printf("\n- Inventario Cargado: %d productos\n", total);
    imprimir_inventario_ordenado(listaInv);

    if (modo == 1) {
        modo_manual(inv, &cola, &params_base);
    }

    liberar_inventario_ordenado(listaInv);
    destruir_hash_inv(inv);
    return 0;
}

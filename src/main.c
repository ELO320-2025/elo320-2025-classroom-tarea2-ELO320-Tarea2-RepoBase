#include "main.h"

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
            printf("\nProcesando imagen: %s\n", nimg->ruta);
            printf("1. Undo último paso\n");
            printf("2. Agregar procesamiento\n");
            printf("3. Buscar código de barras\n");
            printf("4. Abortar imagen\n");
            printf("Seleccione opción: ");

            char buf[64];
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
                printf("Agregar procesamiento:\n");
                printf("1. Umbral de binarización\n");
                printf("2. Ajuste de contraste\n");
                printf("3. Recorte\n");
                printf("4. Rotación\n");
                printf("Seleccione: ");

                if (!fgets(buf, sizeof(buf), stdin)) continue;
                int sub = atoi(buf);

                if (sub == 1) {
                    printf("Nuevo umbral: ");
                    fgets(buf, sizeof(buf), stdin);
                    params_actual.thres = atoi(buf);
                    push_op(&ops, OP_BINARIZACION);
                }

                else if (sub == 2) {
                    int a, b;
                    printf("Nuevo rango (min max): ");
                    fgets(buf, sizeof(buf), stdin);
                    sscanf(buf, "%d %d", &a, &b);
                    params_actual.newMin = a;
                    params_actual.newMax = b;
                    push_op(&ops, OP_CONTRASTE);
                }

                else if (sub == 3) {
                    int x, y, w, h;
                    printf("Recorte (x y w h): ");
                    fgets(buf, sizeof(buf), stdin);
                    sscanf(buf, "%d %d %d %d", &x, &y, &w, &h);
                    params_actual.recX = x;
                    params_actual.recY = y;
                    params_actual.recW = w;
                    params_actual.recH = h;
                    push_op(&ops, OP_RECORTE);
                }

                else if (sub == 4) {
                    printf("Rotación (90 / 180 / 270): ");
                    fgets(buf, sizeof(buf), stdin);
                    int ang = atoi(buf);
                    if (ang == 90) push_op(&ops, OP_ROT_90);
                    else if (ang == 180) push_op(&ops, OP_ROT_180);
                    else if (ang == 270) push_op(&ops, OP_ROT_270);
                    else printf("Ángulo inválido\n");
                }

                liberar_imagen(img_proc, alto_p);
                ancho_p = ancho;
                alto_p = alto;
                img_proc = aplicar_ops_desde_original(
                    img_orig, &ancho_p, &alto_p, &params_actual, &ops
                );
            }

            else if (op == 3) {
                char codigo[14];
                int dv;
                int encontrado = 0;

                for (int k = 30; k <= 70; k += 2) {
                    int y = alto_p * k / 100;
                    int r = decodificar_ean13_desde_grises(
                        img_proc[y], ancho_p, codigo, &dv
                    );
                    if (r == 0 || r == 2) {
                        printf("Código detectado: %s\n", codigo);
                        if (r == 2)
                            printf("Advertencia: dígito verificador incorrecto\n");

                        ProductoInv *p = buscar_inv(inv, codigo);
                        if (p) {
                            printf("Producto: %s\n", p->nombre);
                            printf("Precio: %d\n", p->precio);
                        } else {
                            printf("Código válido pero no inventariado\n");
                        }
                        encontrado = 1;
                        break;
                    }
                }

                if (!encontrado)
                    printf("No se pudo detectar código\n");

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
    if (!inv) {
        printf("Error creando inventario\n");
        return 1;
    }

    int total = 0;
    int cambios = 0;

    NodoProd *lista = cargar_inventario_ordenado_verificado(
        ruta_inv, inv, &total, &cambios
    );

    if (!lista) {
        printf("No se pudo cargar inventario\n");
        destruir_hash_inv(inv);
        return 1;
    }

    if (cambios > 0)
        reescribir_inventario_archivo(ruta_inv, lista);

    Params params_base;
    ColaImagenes cola;

    if (!cargar_config(ruta_cfg, &params_base, &cola)) {
        printf("No se pudo cargar configuración\n");
        liberar_inventario_ordenado(lista);
        destruir_hash_inv(inv);
        return 1;
    }

    printf("Cargando inventario desde '%s'... (%d productos)\n", ruta_inv, total);
    printf("Cargando cola de escaneo desde '%s'... (%d imágenes)\n", ruta_cfg, cola.total);
    printf("\n---\n");

    printf("- Inventario Cargado: %d productos\n", total);
    imprimir_inventario_ordenado(lista);

    printf("\n- Parámetros de Preprocesamiento Inicial:\n");
    imprimir_params(&params_base);
    printf("---\n");

    if (modo == 1) {
        modo_manual(inv, &cola, &params_base);
    } else if (modo == 2) {
        printf("Modo automático no implementado aún\n");
    } else if (modo == 3) {
        printf("Modo semi-automático no implementado aún\n");
    } else {
        printf("Modo inválido\n");
    }

    liberar_inventario_ordenado(lista);
    destruir_hash_inv(inv);
    return 0;
}

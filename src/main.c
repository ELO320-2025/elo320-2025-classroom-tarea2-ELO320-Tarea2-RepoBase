#include "main.h"

static void imprimir_procesamiento(const PilaOps *ops, const Params *p) {
    NodoOp *orden[64];
    int i = ops->size - 1;

    for (NodoOp *n = ops->top; n; n = n->sig)
        orden[i--] = n;

    int idx = 1;
    printf("Procesamiento actual:\n");

    for (i = 0; i < ops->size; i++) {
        NodoOp *op = orden[i];

        if (op->tipo == OP_BINARIZACION)
            printf("%d. Binarizacion Thres=%d\n", idx++, op->valor);

        else if (op->tipo == OP_CONTRASTE)
            printf("%d. Contraste [%d, %d]\n", idx++, p->newMin, p->newMax);

        else if (op->tipo == OP_RECORTE)
            printf("%d. Recorte [%d %d %d %d]\n", idx++, p->recX, p->recY, p->recW, p->recH);

        else if (op->tipo == OP_ROTACION)
            printf("%d. Rotacion %d°\n", idx++, op->valor);
    }
}

static int decodificar_en_imagen(unsigned char **img, int ancho, int alto, char *out_codigo, int *out_dv_img, int *out_r) {
    int dv_img = 0;
    int r = 1;
    int encontrado = 0;

    for (int k = 30; k <= 70; k += 2) {
        int y = alto * k / 100;
        r = decodificar_ean13_desde_grises(img[y], ancho, out_codigo, &dv_img);
        if (r == 0 || r == 2) {
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) return 0;

    if (out_dv_img) *out_dv_img = dv_img;
    if (out_r) *out_r = r;
    return 1;
}

static void modo_manual(HashInv *inv, ColaImagenes *cola, Params params_cfg, Params params_std, ResultadoScan *resultados, int *n_resultados) {
    int total_imgs = cola->total;
    int idx_img = 1;

    while (cola->total > 0) {
        NImg *nimg = pop_cola(cola);
        if (!nimg) break;

        clock_t t_inicio = clock();

        ResultadoScan res;
        memset(&res, 0, sizeof(ResultadoScan));
        res.ruta = dupstr_local(nimg->ruta);
        res.estado = RES_FALLO;
        res.ean[0] = '\0';
        res.producto = NULL;
        res.precio = 0;
        res.intentos = 0;
        res.tiempo_ms = 0.0;

        int ancho0 = 0, alto0 = 0;

        unsigned char **img_orig =leer_ppm_pgm(nimg->ruta, &ancho0, &alto0);

        if (!img_orig) { 
            printf("No se pudo leer imagen %s\n", nimg->ruta); 
            if (res.ruta) { 
                resultados[*n_resultados] = res; 
                (*n_resultados)++; 
            } else { 
                free(res.ruta); 
                free(res.producto); 
            }

            free(nimg->ruta); 
            free(nimg); 
            continue; 
        }

        Params base_img = params_std; 
        base_img.recX = 0; 
        base_img.recY = 0; 
        base_img.recW = ancho0; 
        base_img.recH = alto0; 
        base_img.rot = 0;

        printf("- Parámetros de Preprocesamiento Inicial:\n");
        printf("  - Thres: %d\n", params_std.thres);
        printf("  - Contraste: [%d, %d]\n", params_std.newMin, params_std.newMax);
        printf("  - Recorte: [%d, %d, %d, %d]\n", base_img.recX, base_img.recY, base_img.recW, base_img.recH);
        printf("---\n\n");

        printf("... Procesando imagen %d/%d: %s\n", idx_img++, total_imgs, nimg->ruta);
        printf("Aplicando preprocesamiento estándar...\n\n");

        proceso_interactivo(inv, img_orig, ancho0, alto0, nimg->ruta, params_cfg, params_std, &res);

        clock_t t_fin = clock();
        res.tiempo_ms = (double)(t_fin - t_inicio) * 1000.0 / CLOCKS_PER_SEC;

        resultados[(*n_resultados)++] = res;

        liberar_imagen(img_orig, alto0);
        free(nimg->ruta);
        free(nimg);
        printf("\n");
    }
}

static void modo_automatico(HashInv *inv, ColaImagenes *cola,Params params_cfg, Params params_std, ResultadoScan *resultados, int *n_resultados, int modo, int *algoritmo_usado) {
    int total_imgs = cola->total;
    int idx = 1;

    printf("---\n");
    printf("\nHa seleccionado Modo Automático. Defina la estrategia de evaluación:\n\n");
    printf("Seleccione el Algoritmo de Búsqueda a utilizar:\n");
    printf("1. Algoritmo 1 (Básico)\n");
    printf("2. Algoritmo 2 (Multi-fila)\n");
    printf("3. Algoritmo 3 (Rotaciones)\n");
    printf("> ");

    char buf[32];
    if (!fgets(buf, sizeof(buf), stdin)) return;
    int alg = atoi(buf);
    *algoritmo_usado = alg;

    printf("\nIniciando procesamiento automático con Algoritmo %d y Evaluación Estricta.\n", alg);
    printf("---\n\n");

    while (cola->total > 0) {
        NImg *nimg = pop_cola(cola);
        if (!nimg) break;
        clock_t t_inicio = clock();

        ResultadoScan res;
        memset(&res, 0, sizeof(ResultadoScan));
        res.ruta = dupstr_local(nimg->ruta);
        res.estado = RES_FALLO;
        res.intentos = 0;

        printf("... Procesando imagen %d/%d: %s\n", idx++, total_imgs, nimg->ruta);
        printf("Aplicando preprocesamiento estandar...\n");

        int ancho0, alto0;
        unsigned char **img_orig =
            leer_ppm_pgm(nimg->ruta, &ancho0, &alto0);

        if (!img_orig) {
            printf("ERROR: No se pudo leer imagen\n");
            resultados[(*n_resultados)++] = res;
            free(nimg->ruta);
            free(nimg);
            continue;
        }

        Params base = params_std;
        base.recX = params_cfg.recX;
        base.recY = params_cfg.recY;
        base.recW = params_cfg.recW;
        base.recH = params_cfg.recH;
        base.rot  = 0;

        PilaOps ops;
        init_pila(&ops);
        push_op(&ops, OP_BINARIZACION, base.thres);
        push_op(&ops, OP_CONTRASTE, 0);
        push_op(&ops, OP_RECORTE, 0);

        int ancho = ancho0;
        int alto  = alto0;

        unsigned char **img = aplicar_ops_desde_original(img_orig, &ancho, &alto, &base, &ops);

        char codigo[14] = {0};
        int ok = 0;

        if (alg == 1) {
            ok = algoritmo_1_basico(img, ancho, alto, codigo);
        }

        else if (alg == 2) {

            Params p_local = base;
            PilaOps ops_local;
            init_pila(&ops_local);

            ok = algoritmo_2_autorecorte(img_orig, ancho0, alto0, &p_local, &ops_local, codigo);

            clear_pila(&ops_local);
        }

        else if (alg == 3) {

            PilaOps ops_local;
            init_pila(&ops_local);

            ok = algoritmo_3_rotaciones(img_orig, ancho0, alto0, &params_std, codigo);
            clear_pila(&ops_local);
        }

        if (ok) {
            printf("-> Código EAN-13 decodificado: %s (VÁLIDO).\n", codigo);
            strcpy(res.ean, codigo);

            ProductoInv *p = buscar_inv(inv, codigo);
            if (p) {
                printf("Producto: %s.\n", p->nombre);
                printf("Precio: $%d\n", p->precio);

                res.estado = RES_OK;
                res.producto = dupstr_local(p->nombre);
                res.precio = p->precio;
            } else {
                printf("Producto: No inventariado\n");
                res.estado = RES_NO_INVENTARIADO;
                res.producto = NULL;
                res.precio = 0;
            }
        }

        else {
            if (modo == 3) {
                printf("\n-> ERROR: No se pudo decodificar el código EAN-13.\n");
                printf("FALLO DE DECODIFICACIÓN AUTOMÁTICA. Política: Reprocesamiento Interactivo.\n");
                printf("Cediendo control al usuario para ajustes manuales.\n\n");

                proceso_interactivo(inv, img_orig, ancho0, alto0, nimg->ruta, params_cfg, params_std, &res);
            }
            else {
                printf("-> ERROR: No se pudo decodificar el código EAN-13.\n");
                printf("Política Estricta: Marcando como NO RECONOCIDO.\n");

                res.estado = RES_FALLO;
            }
        }

        clock_t t_fin = clock();
        res.tiempo_ms = (double)(t_fin - t_inicio) * 1000.0 / CLOCKS_PER_SEC;

        resultados[(*n_resultados)++] = res;

        liberar_imagen(img, alto);
        liberar_imagen(img_orig, alto0);
        clear_pila(&ops);

        free(nimg->ruta);
        free(nimg);
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: ./EDAcodebar <modo> <inventario> <config>\n");
        return 1;
    }

    int modo = atoi(argv[1]);
    int algoritmo_usado = 1;

    printf("---EDAcodebar---\n");

    HashInv *inv = crear_hash_inv();
    int total = 0, cambios = 0;

    printf("Cargando inventario desde '%s'... ", argv[2]);
    NodoProd *lista = cargar_inventario_ordenado_verificado(argv[2], inv, &total, &cambios);

    if (!lista) {
        printf("ERROR\n");
        destruir_hash_inv(inv);
        return 1;
    }

    printf("(%d productos)\n", total);

    Params params_cfg;
    ColaImagenes cola;

    printf("Cargando cola de escaneo desde '%s'... ", argv[3]);
    if (!cargar_config(argv[3], &params_cfg, &cola)) {
        printf("ERROR\n");
        liberar_inventario_ordenado(lista);
        destruir_hash_inv(inv);
        return 1;
    }

    printf("(%d imagenes)\n", cola.total);
    printf("\n---\n");

    printf("- Inventario Cargado: %d productos\n", total);
    for (NodoProd *n = lista; n; n = n->sig)
        printf("  - %s (%s)\n", n->nombre, n->codigo);

    printf("- Imagenes en Cola (%d):\n", cola.total);
    for (NImg *n = cola.ini; n; n = n->sig)
        printf("  - %s\n", n->ruta);

    Params params_std;
    params_std.thres  = 100;
    params_std.newMin = 0;
    params_std.newMax = 255;
    params_std.recX   = 0;
    params_std.recY   = 0;
    params_std.recW   = 0;
    params_std.recH   = 0;
    params_std.rot    = 0.0;

    int max_imgs = cola.total;
    ResultadoScan *resultados = malloc(sizeof(ResultadoScan) * max_imgs);
    if (!resultados) {
        printf("Error al reservar memoria para resultados\n");
        liberar_inventario_ordenado(lista);
        destruir_hash_inv(inv);
        return 1;
    }

    int n_resultados = 0;

    if (modo == 1) {
        modo_manual(inv, &cola, params_cfg, params_std, resultados, &n_resultados);
        imprimir_reporte_final(resultados, n_resultados, modo, algoritmo_usado);
    }

    else if (modo == 2) {
        modo_automatico(inv, &cola, params_cfg, params_std, resultados, &n_resultados, modo, &algoritmo_usado);
        imprimir_reporte_final(resultados, n_resultados, modo, algoritmo_usado);
    }

    else if (modo == 3) {
        modo_automatico(inv, &cola, params_cfg, params_std, resultados, &n_resultados, modo, &algoritmo_usado);
        imprimir_reporte_final(resultados, n_resultados, modo, algoritmo_usado);
    }

    else {
        printf("Modo inválido. Use 1 (manual), 2 (automático sin corrección manual) o 3 (automático con corrección manual)\n");
        free(resultados);
        liberar_inventario_ordenado(lista);
        destruir_hash_inv(inv);
        return 1;
    }

    printf("\n... Liberando memoria. Finalizado.\n");

    for (int i = 0; i < n_resultados; i++) {
        free(resultados[i].ruta);
        free(resultados[i].producto);
    }
    free(resultados);
    liberar_inventario_ordenado(lista);
    destruir_hash_inv(inv);
    return 0;
}

int algoritmo_1_basico(unsigned char **img, int ancho, int alto, char *codigo_out) {
    if (!img || !codigo_out || ancho <= 0 || alto <= 0)
        return 0;

    int y = (int)(alto * 0.45);
    if (y < 0) y = 0;
    if (y >= alto) y = alto - 1;

    int dv_img = 0;
    int r = decodificar_ean13_desde_grises(img[y], ancho, codigo_out, &dv_img);

    if (r == 0 || r == 2) {
        return 1;
    }

    return 0;
}

int algoritmo_2_autorecorte(unsigned char **img_orig, int ancho0, int alto0, Params *p, PilaOps *ops, char *codigo_out) {
    int ancho = ancho0;
    int alto  = alto0;

    PilaOps ops_base;
    init_pila(&ops_base);

    push_op(&ops_base, OP_CONTRASTE, 0);
    push_op(&ops_base, OP_BINARIZACION, p->thres);

    unsigned char **img_base =
        aplicar_ops_desde_original(img_orig, &ancho, &alto, p, &ops_base);

    clear_pila(&ops_base);

    if (!img_base)
        return 0;

    for (int y = 0; y + 20 <= alto; y += 5) {

        p->recX = 0;
        p->recY = y;
        p->recW = ancho;
        p->recH = 20;

        int rw = ancho;
        int rh = alto;

        unsigned char **rec =
            img_recortar(img_base, &rw, &rh, p->recX, p->recY, p->recW, p->recH);

        if (rec) {
            if (decodificar_en_imagen(rec, rw, rh, codigo_out, NULL, NULL)) {
                liberar_imagen(rec, rh);
                liberar_imagen(img_base, alto);
                return 1;
            }
            liberar_imagen(rec, rh);
        }
    }

    liberar_imagen(img_base, alto);
    return 0;
}

int algoritmo_3_rotaciones(unsigned char **img_orig, int ancho0, int alto0, Params *params_std, char *codigo_out) {
    if (!img_orig || ancho0 <= 0 || alto0 <= 0 || !params_std || !codigo_out)
        return 0;

    static const int rots[] = {0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330};
    const int n_rots = 12;

    const int STEP_Y = 2;        
    const int MIN_HITS = 3;    

    char mejor_codigo[14] = {0};
    int mejor_hits_validos = 0;
    int mejor_hits_totales = 0;

    for (int i = 0; i < n_rots; i++) {
        int rot = rots[i];

        Params p = *params_std;
        p.rot = rot;

        PilaOps ops;
        init_pila(&ops);

        push_op(&ops, OP_BINARIZACION, p.thres);
        push_op(&ops, OP_CONTRASTE, 0);
        if (rot != 0) push_op(&ops, OP_ROTACION, rot);

        int ancho = ancho0;
        int alto  = alto0;

        unsigned char **img = aplicar_ops_desde_original(img_orig, &ancho, &alto, &p, &ops);
        clear_pila(&ops);

        if (!img) continue;

        char codigo_ref[14] = {0};
        int hits_validos = 0;
        int hits_totales = 0;

        for (int y = 0; y < alto; y += STEP_Y) {
            char tmp[14];
            int dv_calc = 0;

            int r = decodificar_ean13_desde_grises(img[y], ancho, tmp, &dv_calc);
            if (r == 0 || r == 2) {
                hits_totales++;

                if (!ean13_validar(tmp))
                    continue;

                if (hits_validos == 0) {
                    strcpy(codigo_ref, tmp);
                    hits_validos = 1;
                } else if (strcmp(codigo_ref, tmp) == 0) {
                    hits_validos++;
                }
            }
        }

        liberar_imagen(img, alto);

        if (hits_validos > mejor_hits_validos || (hits_validos == mejor_hits_validos && hits_totales > mejor_hits_totales)) {
            mejor_hits_validos = hits_validos;
            mejor_hits_totales = hits_totales;
            strcpy(mejor_codigo, codigo_ref);
        }
    }

    if (mejor_hits_validos >= MIN_HITS) {
        strcpy(codigo_out, mejor_codigo);
        return 1;
    }

    return 0;
}


int proceso_interactivo(HashInv *inv, unsigned char **img_orig, int ancho0, int alto0, const char *ruta_img, Params params_cfg, Params params_std, ResultadoScan *res) {
    Params base_img = params_std;

    base_img.recX = params_cfg.recX;
    base_img.recY = params_cfg.recY;
    base_img.recW = params_cfg.recW;
    base_img.recH = params_cfg.recH;
    base_img.rot  = 0;

    PilaOps ops;
    init_pila(&ops);

    push_op(&ops, OP_BINARIZACION, base_img.thres);
    push_op(&ops, OP_CONTRASTE, 0);
    push_op(&ops, OP_RECORTE, 0);

    int ancho = ancho0;
    int alto  = alto0;

    unsigned char **img = aplicar_ops_desde_original(img_orig, &ancho, &alto, &base_img, &ops);

    if (!img) {
        clear_pila(&ops);
        res->estado = RES_FALLO;
        return 0;
    }

    int salir = 0;

    while (!salir) {
        imprimir_procesamiento(&ops, &base_img);

        printf("\nSeleccione una acción:\n");
        printf("1. Undo último paso\n");
        printf("2. Agregar procesamiento\n");
        printf("3. Buscar código de barras\n");
        printf("4. Abortar y marcar como no reconocido\n> ");

        char buf[64];
        if (!fgets(buf, sizeof(buf), stdin))
            break;

        int op = atoi(buf);

        if (op == 1) {
            if (ops.size > 0) {
                pop_op(&ops);
                liberar_imagen(img, alto);

                ancho = ancho0;
                alto  = alto0;
                img = aplicar_ops_desde_original(img_orig, &ancho, &alto, &base_img, &ops);

                if (!img) {
                    res->estado = RES_FALLO;
                    salir = 1;
                }
            } else {
                printf("No hay más pasos para deshacer\n");
            }
        }

        else if (op == 2) {
            printf("Seleccione nuevo procesamiento a realizar:\n");
            printf("1. Umbral de Binarización (T)\n");
            printf("2. Rango de Contraste\n");
            printf("3. Recorte\n");
            printf("4. Rotación\n> ");

            if (!fgets(buf, sizeof(buf), stdin))
                break;

            int t = atoi(buf);

            if (t == 1) {
                printf("Ingrese nuevo umbral (0-255): ");
                if (!fgets(buf, sizeof(buf), stdin)) break;

                int thr = atoi(buf);
                if (thr < 0) thr = 0;
                if (thr > 255) thr = 255;

                base_img.thres = thr;
                push_op(&ops, OP_BINARIZACION, thr);
            }

            else if (t == 2) {
                int a, b;
                printf("Ingrese newMin newMax (0-255): ");
                if (!fgets(buf, sizeof(buf), stdin)) break;

                if (sscanf(buf, "%d %d", &a, &b) == 2) {
                    if (a < 0) a = 0;
                    if (a > 255) a = 255;
                    if (b < 0) b = 0;
                    if (b > 255) b = 255;

                    base_img.newMin = a;
                    base_img.newMax = b;
                    push_op(&ops, OP_CONTRASTE, 0);
                } else {
                    printf("Entrada inválida\n");
                }
            }

            else if (t == 3) {
                int x, y, w, h;
                printf("Ingrese x y w h: ");
                if (!fgets(buf, sizeof(buf), stdin)) break;

                if (sscanf(buf, "%d %d %d %d", &x, &y, &w, &h) == 4) {
                    base_img.recX = x;
                    base_img.recY = y;
                    base_img.recW = w;
                    base_img.recH = h;
                    push_op(&ops, OP_RECORTE, 0);
                } else {
                    printf("Entrada inválida\n");
                }
            }

            else if (t == 4) {
                printf("Seleccione ángulo de rotacion (en grados enteros [0-360]): ");
                if (!fgets(buf, sizeof(buf), stdin)) break;

                double ang = atof(buf);

                if (ang <= -360.0 || ang >= 360.0) {
                    ang = fmod(ang, 360.0);
                }

                base_img.rot = ang;
                push_op(&ops, OP_ROTACION, ang);
            }

            else {
                printf("Opción inválida\n");
            }

            liberar_imagen(img, alto);
            ancho = ancho0;
            alto  = alto0;

            img = aplicar_ops_desde_original(img_orig, &ancho, &alto, &base_img, &ops);

            if (!img) {
                res->estado = RES_FALLO;
                salir = 1;
            }
        }

        else if (op == 3) {

            char codigo[14];
            int dv_img = 0;
            int r = 1;

            int ok = decodificar_en_imagen(img, ancho, alto, codigo, &dv_img, &r);

            if (!ok) {
                printf("ERROR: No se pudo decodificar el código EAN-13. Patrones de barras no reconocidos.\n");
                res->intentos++;
                res->estado = RES_FALLO;
            } else {
                printf("-> Código EAN-13 decodificado: %s (VÁLIDO).\n", codigo);
                if (r == 2)
                    printf("Advertencia: dígito verificador incorrecto\n");

                strcpy(res->ean, codigo);

                ProductoInv *p = buscar_inv(inv, codigo);
                if (p) {
                    printf("Producto: %s. Precio: $%d\n", p->nombre, p->precio);

                    res->estado = RES_OK;
                    res->producto = dupstr_local(p->nombre);
                    res->precio = p->precio;
                } else {
                    printf("Código válido pero no inventariado\n");

                    res->estado = RES_NO_INVENTARIADO;
                    res->producto = NULL;
                    res->precio = 0;
                }

                salir = 1;
            }
        }

        else if (op == 4) {
            printf("Marcando como NO RECONOCIDO.\n");
            res->estado = RES_ABORTADO;
            salir = 1;
        }

        else {
            printf("Opción inválida\n");
        }
    }

    liberar_imagen(img, alto);
    clear_pila(&ops);
    return (res->estado == RES_OK || res->estado == RES_NO_INVENTARIADO);
}



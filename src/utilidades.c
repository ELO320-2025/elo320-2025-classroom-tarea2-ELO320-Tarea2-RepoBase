#include "utilidades.h"

int ean13_validar(const char *cod) {
    if (!cod) return 0;
    if (strlen(cod) != 13) return 0;

    int i;
    for (i = 0; i < 13; i++) {
        if (cod[i] < '0' || cod[i] > '9')
            return 0;
    }

    int s1 = 0;
    int s2 = 0;

    for (i = 0; i < 12; i++) {
        int v = cod[i] - '0';
        if (i % 2 == 0)
            s1 += v;
        else
            s2 += v;
    }

    int total = s1 + 3 * s2;
    int r = total % 10;
    int dv = (r == 0) ? 0 : 10 - r;

    return dv == (cod[12] - '0');
}

char *dupstr_local(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *r = malloc(n + 1);
    if (!r) return NULL;
    memcpy(r, s, n + 1);
    return r;
}

void imprimir_reporte_final(ResultadoScan *res, int n, int modo, int algoritmo) {

    int total_imgs = n;
    int codigos_validos = 0;
    int reintentos = 0;
    double tiempo_total = 0.0;
    int precio_total = 0;

    printf("Cola de escaneo terminada.\n\n");
    printf("| Imagen | Resultado | EAN-13 | Producto | Precio($)| Intentos | Tiempo (ms) |\n");
    printf("| :--- | :--- | :--- | :--- | :--- | :--- | :--- |\n");

    for (int i = 0; i < n; i++) {
        ResultadoScan *r = &res[i];

        const char *estado_str =
            (r->estado == RES_OK) ? "OK" :
            (r->estado == RES_ABORTADO) ? "ABORTADO" :
            (r->estado == RES_NO_INVENTARIADO) ? "NO INVENTARIADO" :
            "FALLO";

        printf("| %s | %s | %s | %s | %d | %d | %.2f |\n", r->ruta, estado_str, (r->estado == RES_OK || r->estado == RES_NO_INVENTARIADO) ? r->ean : "-", r->producto ? r->producto : "-", r->precio, r->intentos, r->tiempo_ms);

    if (res[i].estado == RES_OK || res[i].estado == RES_NO_INVENTARIADO)
        codigos_validos++;

    reintentos += res[i].intentos;

    tiempo_total += res[i].tiempo_ms;

    if (res[i].estado == RES_OK)
        precio_total += res[i].precio;
    }

    printf("\nResumen:\n");
    if (modo == 1) {
        printf("- Búsqueda manual con Algoritmo %d\n", algoritmo);
    }
    else if (modo == 2) {
        printf("- Búsqueda automática con Algoritmo %d y Evaluación Estricta\n", algoritmo);
    }
    else if (modo == 3) {
        printf("- Búsqueda automática con Algoritmo %d\n", algoritmo);
    }
    printf("- Total de Códigos Válidos Decodificados: %d/%d\n", codigos_validos, total_imgs);
    printf("- Reintentos manuales realizados: %d\n", reintentos);
    printf("- Tiempo promedio de evaluación (ms): %.2f\n", tiempo_total / total_imgs);
    printf("- Precio carrito: $%d\n", precio_total);
}
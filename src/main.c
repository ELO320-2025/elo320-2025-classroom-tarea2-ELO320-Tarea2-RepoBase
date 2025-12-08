#include "main.h"

int main(int argc, char **argv) {
    char ruta[256];
    printf("Ingrese nombre de la imagen: ");
    if (scanf("%255s", ruta) != 1) return 0;

    Imagen *img = leerImagen(ruta);
    if (!img) return 0;

    if (img->tipo == 3) {
        Imagen *g = convertirAGris(img);
        liberarImagen(img);
        img = g;
    }

    Imagen *b = binarizarImagen(img, 100);
    liberarImagen(img);
    img = b;

    int fila = img->alto * 0.40;
    int *linea = malloc(sizeof(int) * img->ancho);
    int i;
    for (i = 0; i < img->ancho; i++) linea[i] = img->gris[fila][i];

    int iniPix = -1;
    int finPix = -1;

    for (i = 0; i < img->ancho; i++) {
        if (linea[i] == 0) { iniPix = i; break; }
    }

    for (i = img->ancho - 1; i >= 0; i--) {
        if (linea[i] == 0) { finPix = i; break; }
    }

    if (iniPix < 0 || finPix <= iniPix) {
        free(linea);
        liberarImagen(img);
        return 0;
    }

    int largoRec = finPix - iniPix + 1;
    int *rec = malloc(sizeof(int) * largoRec);
    for (i = 0; i < largoRec; i++) rec[i] = linea[iniPix + i];
    free(linea);

    SecuenciaBarras s = obtenerSecuenciaBarras(rec, largoRec);
    printf("Recorte: ini=%d fin=%d largo=%d\n", iniPix, finPix, largoRec);
    free(rec);

    SecuenciaBarras n = normalizarSecuencia(s);
    int ini = detectarInicio(n);
    int cen = detectarCentro(n, ini);

    CodigoEAN13 c = decodificarEAN13(n, ini, cen);

    if (c.valido) {
        for (i = 0; i < 13; i++) printf("%d", c.digitos[i]);
        printf("\n");
    }

    free(s.anchos);
    free(n.anchos);
    liberarImagen(img);
    return 0;
}
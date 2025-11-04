#ifndef IMAGEN_H
#define IMAGEN_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    	int w, h, ch;
    	unsigned char *data;
} Imagen;

Imagen *img_crear(int w, int h);

Imagen *img_crear_vertical(int w, int h, int ch);

Imagen *img_copiar(const Imagen *src);

void     img_liberar(Imagen *im);

Imagen *img_cargar(const char *ruta);

Imagen  *img_to_grayscale(const Imagen *src);

Imagen  *img_contrast_stretch(const Imagen *g, int newMin, int newMax);

Imagen  *img_threshold(const Imagen *g, int T);

Imagen *img_franja_central(const Imagen *bin, int alto);

Imagen *img_colapsar_vertical(const Imagen *roi);

Imagen *img_crop(const Imagen *src, int x, int y, int w, int h);

Imagen *img_rot90  (const Imagen *src);

Imagen *img_rot180 (const Imagen *src);

Imagen *img_rot270 (const Imagen *src);


#endif

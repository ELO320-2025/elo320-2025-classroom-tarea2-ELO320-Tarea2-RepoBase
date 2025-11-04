#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/main.h"
#include "../include/preproc.h"
#include "../include/utilidades.h"
#include "../include/imagen.h"
#include "../include/reconocimiento.h"


int app_init(App *app, const char *inventario, const char *config_ruta){
    	if (!app) return 0;
    	memset(app, 0, sizeof(*app));
	app->inventario_ruta = inventario;
	app->config_ruta= config_ruta;

    	printf("Cargando inventario desde '%s'... ", inventario);
    	app->th = cargar_inventario(inventario);
    	if (!app->th){ puts("ERROR"); return 0; }
    	printf("(%d productos)\n", app->th->elementos);

    	printf("Cargando configuración desde '%s'... ", config_ruta);
    	if (!cargar_config(config_ruta, &app->cfg)){
        	puts("ERROR");
        	destruir_TH(app->th);
        	app->th = NULL;
        	return 0;
    	}
    	printf("(%d imágenes)\n\n\n", app->cfg.n_imagenes);
	printf("---\n");
	printf("- Inventario Cargado: %d productos\n",app->th->elementos);
    	imprimir_resumen_inventario(app->th, app->th->elementos);
	printf("- Imagenes en Cola (%d)\n",app->cfg.n_imagenes);
    	imprimir_config(app->config_ruta, &app->cfg);
    	return 1;
}

void app_dispose(App *app){
    	if (!app) return;
    	liberar_config(&app->cfg);
    	destruir_TH(app->th);
    	memset(app, 0, sizeof(*app));
}

int run_manual(App *app){
    	if (!app || !app->th) return 1;
	const Config *cfg = &app->cfg;

    	puts("[MANUAL] ENTER para procesar; 'q' + ENTER para salir.");

    	for (int i=0; i<cfg->n_imagenes; ++i){
        	printf("-> Listo para: %s\n", cfg->imagenes[i]);
        	int ch=getchar(); if (ch=='q'||ch=='Q') break; while(ch!='\n'&&ch!=EOF) ch=getchar();

        	Imagen *orig = img_cargar(cfg->imagenes[i]);
        	if (!orig){ puts("No se pudo abrir la imagen."); continue; }

        	Imagen *g    = img_to_grayscale(orig);
        	Imagen *cimg = img_contrast_stretch(g, cfg->newMin, cfg->newMax);
        	Imagen *b    = img_threshold(cimg, cfg->thres);

        	Imagen *fr   = img_franja_central(b, 12);
        	Imagen *row  = img_colapsar_vertical(fr);

        	img_liberar(orig); img_liberar(g); img_liberar(cimg);
        	img_liberar(b);    img_liberar(fr);

        	if (!row){ puts("No se pudo formar la franja."); continue; }

        	int bits_cap = row->w + 16;
        	int *bits = (int*)malloc((size_t)bits_cap*sizeof(int));
        	int nbits = raster_to_bits(row, bits, bits_cap);

        	char code[14]; int valido=0, ok=0;

        	ok = ean13_decode_from_bits(bits, nbits, code, &valido);

        	if (ok && ean13_verificar_digito(code)) {
            		printf("-> Código EAN-13: %s (VÁLIDO)\n", code);
            		Producto *p = buscar_TH(app->th, code);
            		if (p) printf("Producto: %s. Precio: $%d\n", p->nombre, p->precio);
            		else   puts("Código no encontrado en inventario.");
        	} else {
            		puts("No se pudo decodificar.");
        	}

        	free(bits);
        	img_liberar(row);
        	puts("--");
	}
    	return 0;
}



/*
int run_automatico(App *app){
    (void)app;
    puts("[AUTOMÁTICO] No implementado aún.");
    return 0;
}

int run_hibrido(App *app){
    (void)app;
    puts("[HIBRIDO] No implementado aún.");
    return 0;
}
*/

int main(int argc, char **argv){
    	printf("\n--- EDAcodebar ---\n\n");
    	if (argc < 4){
        	printf("Uso: %s <modo> <inventario.txt> <config.txt>\n", argv[0]);
        	return 1;
    	}

    	int modo = atoi(argv[1]);
    	const char *inventario  = argv[2];
    	const char *config_ruta = argv[3];

    	App app;
    	if (!app_init(&app, inventario, config_ruta)) return 1;

    	int rc = 0;
    	switch (modo){
        	case MODO_MANUAL:      rc = run_manual(&app); break;
		/*
        	case MODO_AUTOMATICO:  rc = run_automatico(&app); break;
        	case MODO_HIBRIDO:     rc = run_hibrido(&app); break;
		*/
        	default:
            		fprintf(stderr, "Modo no válido: %d (usa 1, 2 o 3)\n", modo);
            		rc = 1;
    	}
    	app_dispose(&app);
	liberar_config(&app.cfg);
    	return rc;
}


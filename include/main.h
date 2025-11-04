#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "preproc.h"
#include "utilidades.h"

#define MODO_MANUAL      1
/*
#define MODO_AUTOMATICO  2
#define MODO_HIBRIDO     3
*/
typedef struct {
    	TablaHash *th;
    	Config cfg;
	const char *inventario_ruta;
    	const char *config_ruta;
} App;

int  app_init(App *app, const char *inventario, const char *config_ruta);
void app_dispose(App *app);

int run_manual(App *app);

/*
int run_automatico(App *app);
int run_hibrido(App *app);
*/

#endif


#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "imagen.h"
#include "preproc.h"
#include "utilidades.h"
#include "reconocimiento.h"

int algoritmo_1_basico(unsigned char **img, int ancho, int alto, char *codigo_out);

int algoritmo_2_autorecorte(unsigned char **img_orig, int ancho0, int alto0, Params *p, PilaOps *ops, char *codigo_out);

int algoritmo_3_rotaciones(unsigned char **img_orig, int ancho0, int alto0, Params *params_std, char *codigo_out);

int proceso_interactivo(HashInv *inv, unsigned char **img_orig, int ancho0, int alto0, const char *ruta_img, Params params_cfg, Params params_std, ResultadoScan *res);

#endif
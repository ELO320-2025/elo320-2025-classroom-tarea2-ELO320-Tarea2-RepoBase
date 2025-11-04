
#include "../include/utilidades.h"

void trim_inplace(char *s){
    	size_t n = strlen(s);
    	size_t i = 0;
    	while (i<n && (s[i]==' '||s[i]=='\t'||s[i]=='\r'||s[i]=='\n')) i++;
    	size_t j = n;
    	while (j>i && (s[j-1]==' '||s[j-1]=='\t'||s[j-1]=='\r'||s[j-1]=='\n')) j--;
    	if (i>0) memmove(s, s+i, j-i);
    	s[j-i] = '\0';
}

char *duplicar_cadena(const char *s){
    	if (!s) return NULL;
    	size_t n = strlen(s);
    	char *p = (char*)malloc(n+1);
    	if (!p) return NULL;
    	memcpy(p, s, n+1);
    	return p;
}

int esta_vacia(const char *s){
    	if (!s) return 1;
    	while (*s){
        	if (!(*s==' '||*s=='\t'||*s=='\r'||*s=='\n')) return 0;
        	s++;
    	}
    	return 1;
}

int cargar_config(const char *ruta, Config *cfg){
    	if (!ruta || !cfg) return 0;
    	FILE *f = fopen(ruta, "r");
    	if (!f) return 0;

    	cfg->thres = 100;
    	cfg->newMin = 0;
    	cfg->newMax = 255;
    	cfg->recorte_linea[0]=0; cfg->recorte_linea[1]=0; cfg->recorte_linea[2]=200; cfg->recorte_linea[3]=100;
    	cfg->recorte_rect[0]=0; cfg->recorte_rect[1]=0; cfg->recorte_rect[2]=0; cfg->recorte_rect[3]=0;
    	cfg->imagenes = NULL;
    	cfg->n_imagenes = 0;

    	char linea[4096];
    	int en_lista = 0;

    	while (fgets(linea, sizeof(linea), f)){
        	trim_inplace(linea);
        	if (!*linea) continue;
        	if (linea[0]=='#') continue;



        	if (!en_lista) {
            		int a,b,c,d;
            		if (sscanf(linea, "Thres %d", &cfg->thres) == 1) continue;

            		if (sscanf(linea, "newMin %d newMax %d", &cfg->newMin, &cfg->newMax) == 2) {
                		if (cfg->newMin < 0) cfg->newMin = 0;
                		if (cfg->newMax > 255) cfg->newMax = 255;
                		if (cfg->newMax <= cfg->newMin) cfg->newMax = cfg->newMin + 1;
            	    		continue;
            		}

            		if (sscanf(linea, "Recorte %d %d %d %d", &a, &b, &c, &d) == 4) {
                		cfg->recorte_linea[0] = a;
                		cfg->recorte_linea[1] = b;
                		cfg->recorte_linea[2] = c;
                		cfg->recorte_linea[3] = d;
                		continue;
            		}

            		if (!strcasecmp(linea, "Imagenes") || !strcasecmp(linea, "Imágenes")) {
                		en_lista = 1;
                		continue;
            		}

            		continue;
        	} else {
            		char *path = duplicar_cadena(linea);
            		if (!path) { fclose(f); liberar_config(cfg); return 0; }

            		char **nuevo = (char**)realloc(cfg->imagenes, (size_t)(cfg->n_imagenes + 1) * sizeof(char*));
            		if (!nuevo) { free(path); fclose(f); liberar_config(cfg); return 0; }

            		cfg->imagenes = nuevo;
            		cfg->imagenes[cfg->n_imagenes++] = path;
        	}
    	}

    	fclose(f);

    	return (cfg->n_imagenes > 0) ? 1 : 0;
}

void liberar_config(Config *cfg){
    	if (!cfg) return;
    	for (int i=0;i<cfg->n_imagenes;i++) free(cfg->imagenes[i]);
    	free(cfg->imagenes);
    	cfg->imagenes = NULL;
    	cfg->n_imagenes = 0;
}

void imprimir_config(const char *ruta, const Config *cfg){
    	if (!cfg) return;
    	printf(" - Thres: %d\n", cfg->thres);
    	printf(" - Contraste: [%d, %d]\n", cfg->newMin, cfg->newMax);
    	printf(" - Recorte (linea): [%d %d %d %d]\n",
           	cfg->recorte_linea[0], cfg->recorte_linea[1], cfg->recorte_linea[2], cfg->recorte_linea[3]);
    	printf(" - Recorte (rect):  [%d %d %d %d]\n",
           	cfg->recorte_rect[0], cfg->recorte_rect[1], cfg->recorte_rect[2], cfg->recorte_rect[3]);
    	printf("---\n");
}


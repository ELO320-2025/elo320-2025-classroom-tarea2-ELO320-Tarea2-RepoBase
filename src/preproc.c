#include "../include/preproc.h"

static size_t hash_codigo(const char *s){
    	size_t h = 1469598103934665603ULL;
    	while (*s){ h ^= (unsigned char)(*s++); h *= 1099511628211ULL; }
    	return h;
}

TablaHash *crear_TH(size_t capacidad){
    	if (capacidad < 16) capacidad = 16;
    	TablaHash *th = (TablaHash*)malloc(sizeof(TablaHash));
    	if (!th) return NULL;
    	th->buckets = (Producto**)calloc(capacidad, sizeof(Producto*));
    	if (!th->buckets){ free(th); return NULL; }
    	th->capacidad = capacidad;
    	th->elementos = 0;
	th->next_orden = 0;
    	return th;
}

void destruir_TH(TablaHash *th){
    	if (!th) return;
    	for (size_t i=0;i<th->capacidad;i++){
        	Producto *p = th->buckets[i];
        	while (p){
            		Producto *q = p->sig;
            		free(p->codigo);
            		free(p->nombre);
            		free(p);
            		p = q;
        	}
    	}
    	free(th->buckets);
    	free(th);
}

int insertar_TH(TablaHash *th, const char *codigo, const char *nombre, int precio){
    	if (!th || !codigo || !nombre) return 0;
    	size_t idx = hash_codigo(codigo) % th->capacidad;
    	for (Producto *p = th->buckets[idx]; p; p = p->sig){
        	if (strcmp(p->codigo, codigo) == 0){
            		free(p->nombre);
            		p->nombre = (char*)malloc(strlen(nombre)+1);
            		if (!p->nombre) return 0;
            		strcpy(p->nombre, nombre);
            		p->precio = precio;
            		return 1;
        	}
    	}
    	Producto *n = (Producto*)malloc(sizeof(Producto));
    	if (!n) return 0;
    	n->codigo = (char*)malloc(strlen(codigo)+1);
    	n->nombre = (char*)malloc(strlen(nombre)+1);
    	if (!n->codigo || !n->nombre){ free(n->codigo); free(n->nombre); free(n); return 0; }
    	strcpy(n->codigo, codigo);
    	strcpy(n->nombre, nombre);
    	n->precio = precio;
	n->orden = th->next_orden++;
    	n->sig = th->buckets[idx];
    	th->buckets[idx] = n;
    	th->elementos++;
    	return 1;
}

Producto *buscar_TH(TablaHash *th, const char *codigo){
    	if (!th || !codigo) return NULL;
    	size_t idx = hash_codigo(codigo) % th->capacidad;
    	for (Producto *p = th->buckets[idx]; p; p = p->sig){
        	if (strcmp(p->codigo, codigo) == 0) return p;
    	}
    	return NULL;
}

static void _trim(char *s){
    	size_t n = strlen(s);
    	size_t i = 0;
    	while (i<n && (s[i]==' '||s[i]=='\t'||s[i]=='\r'||s[i]=='\n')) i++;
    	size_t j = n;
    	while (j>i && (s[j-1]==' '||s[j-1]=='\t'||s[j-1]=='\r'||s[j-1]=='\n')) j--;
   	if (i>0) memmove(s, s+i, j-i);
    	s[j-i] = '\0';
}

TablaHash *cargar_inventario(const char *ruta){
    	FILE *f = fopen(ruta, "r");
    	if (!f) return NULL;
    	TablaHash *th = crear_TH(257);
    	if (!th){ fclose(f); return NULL; }
    	char linea[4096];
    	while (fgets(linea, sizeof(linea), f)){
        	if (!*linea) continue;
        	if (linea[0]=='#') continue;
        	char *p1 = strchr(linea, '|');
        	if (!p1) continue;
        	char *p2 = strchr(p1+1, '|');
        	if (!p2) continue;
        	*p1 = '\0';
        	*p2 = '\0';
        	char *nombre = linea;
        	char *precio_s = p1+1;
        	char *codigo = p2+1;
        	_trim(nombre);
        	_trim(precio_s);
        	_trim(codigo);
        	if (!*codigo || !*precio_s || !*nombre) continue;
        	int precio = atoi(precio_s);
        	insertar_TH(th, codigo, nombre, precio);
    	}
    	fclose(f);
    	return th;
}


static int cmp_por_orden(const void *a, const void *b){
    	const Producto *pa = *(const Producto * const *)a;
    	const Producto *pb = *(const Producto * const *)b;
    	if (pa->orden < pb->orden) return -1;
    	if (pa->orden > pb->orden) return 1;
    	return 0;
}




void imprimir_resumen_inventario(TablaHash *th, int max_items){
    	if (!th) return;
    	int count = 0;

    	Producto **vec = malloc(th->elementos * sizeof(Producto*));
    	if (!vec) return;

    	int k = 0;
    	for (size_t i = 0; i < th->capacidad; i++){
        	for (Producto *p = th->buckets[i]; p; p = p->sig)
            	vec[k++] = p;
    	}

    	qsort(vec, th->elementos, sizeof(Producto*), cmp_por_orden);

    	for (int i = 0; i < th->elementos; i++){
        	printf(" - %s (%s)\n", vec[i]->nombre, vec[i]->codigo);
        	if (max_items > 0 && ++count >= max_items) break;
    	}
    	free(vec);
}


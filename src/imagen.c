#include "../include/imagen.h"

static int _isspace_c(int c){ return c==' '||c=='\t'||c=='\r'||c=='\n'; }


static void _skip_ws_and_comments(FILE *f){
    	int c = fgetc(f);
    	for (;;) {
        	while (c=='#'){ while (c!='\n' && c!=EOF) c=fgetc(f); c=fgetc(f); }
        	if (c==EOF) return;
        	if (!_isspace_c(c)) { ungetc(c, f); return; }
        	c = fgetc(f);
    	}
}

static int _read_token(FILE *f, char *tok, int n){
    	int c, i = 0;
    	_skip_ws_and_comments(f);
    	c = fgetc(f);
    	if (c==EOF) return 0;
    	while (!_isspace_c(c) && c!=EOF) {
        	if (i < n-1) tok[i++] = (char)c;
        	c = fgetc(f);
    	}
    	tok[i] = '\0';
    	return i>0;
}

static Imagen *_alloc_img(int w, int h){
    	Imagen *im = (Imagen*)malloc(sizeof(Imagen));
    	if (!im) return NULL;
    	im->w = w; im->h = h; im->ch = 1;
    	im->data = (unsigned char*)malloc((size_t)w*h);
    	if (!im->data){ free(im); return NULL; }
    	return im;
}

Imagen *img_crear(int w, int h){
    	if (w <= 0 || h <= 0) return NULL;

    	size_t N = (size_t)w * (size_t)h;
    	if (N / (size_t)w != (size_t)h) return NULL;

    	Imagen *im = (Imagen *)malloc(sizeof(Imagen));
    	if (!im) return NULL;

    	im->w = w;
    	im->h = h;
    	im->data = (unsigned char *)calloc(N, sizeof(unsigned char));
    	if (!im->data){ free(im); return NULL; }

    	return im;
}

Imagen *img_copiar(const Imagen *src){
    	if (!src || !src->data) return NULL;
    	size_t N = (size_t)src->w * (size_t)src->h;

    	Imagen *dst = img_crear(src->w, src->h);
    	if (!dst) return NULL;

    	memcpy(dst->data, src->data, N);
    	return dst;
}




void img_liberar(Imagen *im){
    	if (!im) return;
    	free(im->data);
    	free(im);
}

static unsigned char _rgb_to_gray(unsigned char r, unsigned char g, unsigned char b){
    	int y = (int)(0.299*r + 0.587*g + 0.114*b + 0.5);
    	if (y < 0) y = 0;
	if (y > 255) y = 255;
    	return (unsigned char)y;
}

Imagen *img_cargar(const char *ruta){
    	FILE *f = fopen(ruta, "rb");
    	if (!f) return NULL;

    	char id[3] = {0};
    	if (fread(id,1,2,f)!=2){ fclose(f); return NULL; }
    	if (id[0] != 'P'){ fclose(f); return NULL; }

    	_skip_ws_and_comments(f);
    	char tok[64];
    	if (!_read_token(f, tok, sizeof(tok))){ fclose(f); return NULL; }
    	int w = atoi(tok);
    	if (!_read_token(f, tok, sizeof(tok))){ fclose(f); return NULL; }
    	int h = atoi(tok);
    	if (!_read_token(f, tok, sizeof(tok))){ fclose(f); return NULL; }
    	int maxv = atoi(tok);
    	if (w<=0 || h<=0 || maxv<=0){ fclose(f); return NULL; }

    	Imagen *im = _alloc_img(w,h);
    	if (!im){ fclose(f); return NULL; }

    	if (id[1]=='2' || id[1]=='5'){
        	if (id[1]=='2'){
            		for (int i=0;i<w*h;i++){
                		if (!_read_token(f, tok, sizeof(tok))){ img_liberar(im); fclose(f); return NULL; }
                		int v = atoi(tok);
                		if (v<0) v=0;
				if (v>maxv) v=maxv;
                		im->data[i] = (unsigned char)((255*v)/maxv);
            		}
        	} else {
            		size_t need = (size_t)w*h;
            		if (fread(im->data,1,need,f)!=need){ img_liberar(im); fclose(f); return NULL; }
            		if (maxv != 255){
                		for (int i=0;i<w*h;i++){
                    			im->data[i] = (unsigned char)((255*im->data[i])/maxv);
                		}
            		}
        	}
    	} else if (id[1]=='3' || id[1]=='6'){
        	if (id[1]=='3'){
            		for (int i=0;i<w*h;i++){
                		if (!_read_token(f,tok,sizeof(tok))){ img_liberar(im); fclose(f); return NULL; }
                		int r = atoi(tok);
                		if (!_read_token(f,tok,sizeof(tok))){ img_liberar(im); fclose(f); return NULL; }
                		int g = atoi(tok);
                		if (!_read_token(f,tok,sizeof(tok))){ img_liberar(im); fclose(f); return NULL; }
                		int b = atoi(tok);
                		if (r<0) r=0;
				if (g<0) g=0;
				if (b<0) b=0;
                		if (r>maxv) r=maxv;
				if (g>maxv) g=maxv;
				if (b>maxv) b=maxv;
                		r = (255*r)/maxv; g = (255*g)/maxv; b = (255*b)/maxv;
                		im->data[i] = _rgb_to_gray((unsigned char)r,(unsigned char)g,(unsigned char)b);
            		}
        	} else {
            		size_t need = (size_t)w*h*3;
            		unsigned char *tmp = (unsigned char*)malloc(need);
            		if (!tmp){ img_liberar(im); fclose(f); return NULL; }
            		if (fread(tmp,1,need,f)!=need){ free(tmp); img_liberar(im); fclose(f); return NULL; }
            		if (maxv != 255){
                		for (size_t i=0;i<need;i++) tmp[i] = (unsigned char)((255*tmp[i])/maxv);
            		}
            		for (int i=0;i<w*h;i++){
                		im->data[i] = _rgb_to_gray(tmp[3*i+0], tmp[3*i+1], tmp[3*i+2]);
            		}
            		free(tmp);
        	}
    	} else {
        	img_liberar(im); fclose(f); return NULL;
    	}

    	fclose(f);
    	return im;
}

Imagen *img_to_grayscale(const Imagen *src){
    	if (!src) return NULL;
    	Imagen *d = _alloc_img(src->w, src->h);
    	if (!d) return NULL;
    	memcpy(d->data, src->data, (size_t)src->w*src->h);
    	return d;
}

Imagen *img_contrast_stretch(const Imagen *src, int newMin, int newMax){
    	if (!src || src->ch!=1) return NULL;
    	if (newMin<0) newMin=0;
	if (newMin>255) newMin=255;
    	if (newMax<0) newMax=0;
	if (newMax>255) newMax=255;
    	if (newMax <= newMin) newMax = newMin + 1;

    	int lo = 255, hi = 0;
    	int N = src->w*src->h;
    	for (int i=0;i<N;i++){
        	int v = src->data[i];
        	if (v < lo) lo = v;
        	if (v > hi) hi = v;
    	}
    	if (hi == lo) { lo = 0; hi = 1; }
    	Imagen *d = _alloc_img(src->w, src->h);
    	if (!d) return NULL;
    	for (int i=0;i<N;i++){
        	int v = src->data[i];
        	int y = newMin + (v - lo) * (newMax - newMin) / (hi - lo);
        	if (y<0) y=0;
		if (y>255) y=255;
        	d->data[i] = (unsigned char)y;
    	}
    	return d;
}

Imagen *img_threshold(const Imagen *src, int thres){
    	if (!src || src->ch!=1) return NULL;
    	if (thres < 0) thres = 0;
	if (thres > 255) thres = 255;
    	Imagen *d = _alloc_img(src->w, src->h);
    	if (!d) return NULL;
    	int N = src->w*src->h;
    	for (int i=0;i<N;i++){
        	d->data[i] = (unsigned char)(src->data[i] >= thres ? 255 : 0);
    	}
    	return d;
}

Imagen *img_crear_vertical(int w, int h, int ch){
    	Imagen *img = (Imagen*)malloc(sizeof(Imagen));
    	if(!img) return NULL;
    	img->w = w; img->h = h; img->ch = ch;
    	size_t bytes = (size_t)w * h * ch;
    	img->data = (unsigned char*)calloc(bytes, 1);
    	if(!img->data){ free(img); return NULL; }
    	return img;
}


Imagen *img_crop(const Imagen *src, int x, int y, int w, int h){
    	if (!src || src->ch!=1) return NULL;
    	if (w < 1 || h < 1) return NULL;
    	if (x < 0) { w += x; x = 0; }
    	if (y < 0) { h += y; y = 0; }
    	if (x + w > src->w) w = src->w - x;
    	if (y + h > src->h) h = src->h - y;
    	if (w < 1 || h < 1) return NULL;

    	Imagen *d = _alloc_img(w, h);
    	if (!d) return NULL;
    	for (int j=0;j<h;j++){
    		memcpy(d->data + (size_t)j*w,
               	src->data + (size_t)(y+j)*src->w + x,
               	(size_t)w);
    	}
    	return d;
}


static inline unsigned char clamp_u8(int v){
    	if (v < 0)   return 0;
    	if (v > 255) return 255;
    	return (unsigned char)v;
}

Imagen *img_rot0(const Imagen *src){
    	if (!src) return NULL;
    	return img_copiar(src);
}

Imagen *img_rot90(const Imagen *src){
    	if (!src) return NULL;
    	int w = src->w, h = src->h;
    	Imagen *dst = img_crear(h, w);
   	 if (!dst) return NULL;
    	for (int y = 0; y < h; ++y){
        	const unsigned char *row = &src->data[y*w];
        	for (int x = 0; x < w; ++x){
            		int dx = h - 1 - y;
            		int dy = x;
            		dst->data[dy*dst->w + dx] = row[x];
        	}
    	}
    	return dst;
}

Imagen *img_rot180(const Imagen *src){
    	if (!src) return NULL;
    	int w = src->w, h = src->h;
    	Imagen *dst = img_crear(w, h);
    	if (!dst) return NULL;
    	for (int y = 0; y < h; ++y){
        	for (int x = 0; x < w; ++x){
            		int dx = w - 1 - x;
            		int dy = h - 1 - y;
            		dst->data[dy*w + dx] = src->data[y*w + x];
        	}
    	}
    	return dst;
}

Imagen *img_rot270(const Imagen *src){
    	if (!src) return NULL;
    	int w = src->w, h = src->h;
    	Imagen *dst = img_crear(h, w);
    	if (!dst) return NULL;
    	for (int y = 0; y < h; ++y){
        	const unsigned char *row = &src->data[y*w];
        	for (int x = 0; x < w; ++x){
            		int dx = y;
            		int dy = w - 1 - x;
            		dst->data[dy*dst->w + dx] = row[x];
        	}
    	}
    	return dst;
}

Imagen *img_contraste(const Imagen *src, int newMin, int newMax){
    	if (!src) return NULL;
    	if (newMin < 0) newMin = 0;
    	if (newMax > 255) newMax = 255;
    	if (newMax < newMin) { int t = newMax; newMax = newMin; newMin = t; }

    	int w = src->w, h = src->h, N = w*h;
    	const unsigned char *s = src->data;

    	int Imin = 255, Imax = 0;
    	for (int i = 0; i < N; ++i){
        	int v = s[i];
        	if (v < Imin) Imin = v;
        	if (v > Imax) Imax = v;
    	}

   	Imagen *dst = img_crear(w, h);
    	if (!dst) return NULL;

    	if (Imax == Imin){
        	unsigned char fill = clamp_u8((newMin + newMax)/2);
        	memset(dst->data, fill, (size_t)N);
        	return dst;
    	}

    	double scale = (double)(newMax - newMin) / (double)(Imax - Imin);
    	for (int i = 0; i < N; ++i){
		double vreal = (s[i] - Imin) * scale + newMin;
        	int v = (int)(vreal + 0.5);
		if (v < 0) v = 0;
        	if (v > 255) v = 255;
        	dst->data[i] = clamp_u8(v);
    	}
    	return dst;
}

/*
static void _swap_int(int *a,int *b){ int t=*a; *a=*b; *b=t; }
*/
Imagen *img_franja_central(const Imagen *src, int altura){
    	if (!src || altura < 1) return NULL;
    	int y0 = src->h/2 - altura/2;
    	if (y0 < 0) y0 = 0;
    	if (y0 + altura > src->h) altura = src->h - y0;
    	return img_crop((Imagen*)src, 0, y0, src->w, altura);
}

Imagen *img_colapsar_vertical(const Imagen *bin){
    	if (!bin || bin->ch != 1 || bin->h < 1) return NULL;
    	Imagen *row = img_crear_vertical	(bin->w, 1, 1);
    	if (!row) return NULL;

    	for (int x = 0; x < bin->w; x++){
        	int acc = 0;
        	for (int y = 0; y < bin->h; y++)
            	acc += bin->data[y*bin->w + x];
        	row->data[x] = (unsigned char)(acc / bin->h);
    	}
    	return row;
}



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "../include/imagen.h"
#include "../include/reconocimiento.h"

int raster_to_bits(const Imagen *row, int *bits, int max_bits){
    	if (!row || row->h!=1 || row->ch!=1) return 0;
    	if (max_bits < row->w) max_bits = row->w;
    	long acc=0; for (int x=0; x<row->w; x++) acc += row->data[x];
    	int T = (int)(acc / (row->w? row->w:1));
    	int n=0;
    	for (int x=0; x<row->w && n<max_bits; x++){
        	bits[n++] = (row->data[x] < T) ? 1 : 0;
    	}
    	return n;
}

static int runs_from_bits(const int *b, int n, int *runs, int max_runs){
    	if (n<=0) return 0;
    	int cur=b[0], len=1, k=0;
    	for (int i=1;i<n;i++){
        	if (b[i]==cur) len++;
        	else { if (k<max_runs) runs[k++]=len; cur=b[i]; len=1; }
    	}
    	if (k<max_runs) runs[k++]=len;
    	return k;
}

static double promedio(const int *a, int n){
	double s=0; for(int i=0;i<n;i++) s+=a[i]; return n? s/n : 1.0;
}

int ean13_decode_from_bits(const int *bits, int nbits, char out_code[14], int *valido){
    	int runs[4096]; int nr = runs_from_bits(bits, nbits, runs, 4096);
    	if (nr < 95/2) return 0;

    	double m = promedio(runs, nr)/2.0; if (m<=0) m=1;

    	int b95[95]; int p=0; double acc=0;
    	for (int i=0, color=bits[0]; i<nr && p<95; i++, color^=1){
        	acc += runs[i];
        	while (acc >= m && p<95){
            		b95[p++] = color;
            		acc -= m;
        	}
    	}
    	if (p<95) while(p<95) b95[p]=b95[p? p-1:0], p++;

    	if (!(b95[0]==1 && b95[1]==0 && b95[2]==1)) return 0;
    	if (!(b95[45]==0&&b95[46]==1&&b95[47]==0&&b95[48]==1&&b95[49]==0)) return 0;
    	if (!(b95[92]==1&&b95[93]==0&&b95[94]==1)) return 0;

    	(void)out_code; (void)valido;
    	return 0;
}

int ean13_verificar_digito(const char code[14]){
    	int suma=0;
    	for(int i=0;i<12;i++){
        	int d = code[i]-'0';
        	suma += (i%2)? 3*d : d;
    	}
    	int dig = (10 - (suma%10))%10;
    	return dig == (code[12]-'0');
}


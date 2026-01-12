#include "reconocimiento.h"

#include "reconocimiento.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static const char *EAN_L[10] = {
 "0001101","0011001","0010011","0111101","0100011",
 "0110001","0101111","0111011","0110111","0001011"
};
static const char *EAN_G[10] = {
 "0100111","0110011","0011011","0100001","0011101",
 "0111001","0000101","0010001","0001001","0010111"
};
static const char *EAN_R[10] = {
 "1110010","1100110","1101100","1000010","1011100",
 "1001110","1010000","1000100","1001000","1110100"
};
static const char *EAN_PARITY[10] = {
 "LLLLLL","LLGLGG","LLGGLG","LLGGGL","LGLLGG",
 "LGGLLG","LGGGLL","LGLGLG","LGLGGL","LGGLGL"
};

static int calcular_dv(const char *cod13){
    int s1 = 0, s2 = 0;
    for (int i = 0; i < 12; i++) {
        int v = cod13[i] - '0';
        if ((i % 2) == 0) s1 += v;
        else s2 += v;
    }
    int t = s1 + 3 * s2;
    int r = t % 10;
    return (r == 0) ? 0 : (10 - r);
}

static int hamming7(const char *a, const char *b){
    int d = 0;
    for (int i = 0; i < 7; i++)
        if (a[i] != b[i]) d++;
    return d;
}

static void sort_cands(cand *c, int n){
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (c[j].e < c[i].e) {
                cand t = c[i];
                c[i] = c[j];
                c[j] = t;
            }
        }
    }
}

static int candidatos(const char *bits, int izq, cand *c){
    int best = 3;
    int n = 0;

    if (izq) {
        for (int d = 0; d < 10; d++) {
            int eL = hamming7(bits, EAN_L[d]);
            int eG = hamming7(bits, EAN_G[d]);
            if (eL < best) best = eL;
            if (eG < best) best = eG;
        }
    } else {
        for (int d = 0; d < 10; d++) {
            int eR = hamming7(bits, EAN_R[d]);
            if (eR < best) best = eR;
        }
    }

    if (best > 2) return 0;

    if (izq) {
        for (int d = 0; d < 10; d++) {
            int eL = hamming7(bits, EAN_L[d]);
            if (eL == best || eL == best + 1) c[n++] = (cand){ d, 'L', eL };

            int eG = hamming7(bits, EAN_G[d]);
            if (eG == best || eG == best + 1) c[n++] = (cand){ d, 'G', eG };
        }
    } else {
        for (int d = 0; d < 10; d++) {
            int eR = hamming7(bits, EAN_R[d]);
            if (eR == best || eR == best + 1) c[n++] = (cand){ d, 'R', eR };
        }
    }

    sort_cands(c, n);
    if (n > 8) n = 8;
    return n;
}

static int obtener_primer_digito(const char *par6){
    for (int i = 0; i < 10; i++)
        if (!strcmp(par6, EAN_PARITY[i])) return i;
    return -1;
}

static int guardas_ok_bits95(const char *b){
    if(!(b[0]=='1' && b[1]=='0' && b[2]=='1')) return 0;
    if(!(b[45]=='0'&&b[46]=='1'&&b[47]=='0'&&b[48]=='1'&&b[49]=='0')) return 0;
    if(!(b[92]=='1' && b[93]=='0' && b[94]=='1')) return 0;
    return 1;
}

static int umbral_histograma_simple(const unsigned char *fila, int ancho){
    int hist[256]={0};
    for(int i=0;i<ancho;i++) hist[fila[i]]++;
    int pico=0;
    for(int i=1;i<256;i++) if(hist[i]>hist[pico]) pico=i;
    int th=pico-40;
    if(th<20) th=20;
    if(th>220) th=220;
    return th;
}

static void remuestrear_95_desde_intervalo(const int *bin, int start, int end, char *bits95){
    double W = (double)(end - start);
    for(int i=0;i<95;i++){
        double pos = (double)start + (i + 0.5) * (W / 95.0);
        int x = (int)pos;
        bits95[i] = bin[x] ? '1' : '0';
    }
    bits95[95]='\0';
}

static int decode_left_exact(const char *p7, char *par_out, const char **EAN_L, const char **EAN_G){
    for(int d=0; d<10; d++){
        if(!strcmp(p7, EAN_L[d])){ *par_out='L'; return d; }
        if(!strcmp(p7, EAN_G[d])){ *par_out='G'; return d; }
    }
    return -1;
}

static int decode_right_exact(const char *p7, const char **EAN_R){
    for(int d=0; d<10; d++){
        if(!strcmp(p7, EAN_R[d])) return d;
    }
    return -1;
}

static int intentar_decodificar_bits95(const char *bits95, const char **EAN_L, const char **EAN_G, const char **EAN_R, const char **EAN_PARITY, int (*calcular_dv)(const char*), int (*obtener_primer_digito)(const char*), int (*candidatos_fn)(const char*, int, cand*), char *out, int *dv_out){
    char cod[14];
    char par[7];
    par[6]='\0';

    cand cand_list[13][8];
    int  cand_n[13];
    for(int i=0;i<13;i++) cand_n[i]=0;

    int dudosos[13];
    int ndud=0;

    for(int d=0; d<6; d++){
        int off = 3 + 7*d;
        char p7[8];
        for(int t=0;t<7;t++) p7[t]=bits95[off+t];
        p7[7]='\0';

        char pin=0;
        int val = decode_left_exact(p7, &pin, EAN_L, EAN_G);
        if(val >= 0){
            cod[1+d] = (char)('0'+val);
            par[d] = pin;

            cand_list[1+d][0] = (cand){val, pin, 0};
            cand_n[1+d] = 1;
        }else{
            cand tmp[32];
            int nc = candidatos_fn(p7, 1, tmp);
            if(nc <= 0) return 1;
            int keep = (nc>8)?8:nc;
            for(int i=0;i<keep;i++) cand_list[1+d][i] = tmp[i];
            cand_n[1+d] = keep;

            cod[1+d] = (char)('0' + cand_list[1+d][0].d);
            par[d]   = cand_list[1+d][0].p;

            dudosos[ndud++] = 1+d;
        }
    }

    for(int d=0; d<6; d++){
        int off = 50 + 7*d;
        char p7[8];
        for(int t=0;t<7;t++) p7[t]=bits95[off+t];
        p7[7]='\0';

        int val = decode_right_exact(p7, EAN_R);
        if(val >= 0){
            cod[7+d] = (char)('0'+val);
            cand_list[7+d][0] = (cand){val,'R',0};
            cand_n[7+d] = 1;
        }else{
            cand tmp[32];
            int nc = candidatos_fn(p7, 0, tmp);
            if(nc <= 0) return 1;
            int keep = (nc>8)?8:nc;
            for(int i=0;i<keep;i++) cand_list[7+d][i] = tmp[i];
            cand_n[7+d] = keep;

            cod[7+d] = (char)('0' + cand_list[7+d][0].d);

            int pos = 7+d;
            if(pos != 12) dudosos[ndud++] = pos; 
        }
    }

    int fd = obtener_primer_digito(par);
    if(fd < 0) return 1;

    cod[0]  = (char)('0' + fd);
    cod[13] = '\0';

    int dv_calc = calcular_dv(cod);
    *dv_out = dv_calc;

    if(dv_calc == (cod[12]-'0')){
        strcpy(out, cod);
        return 0;
    }

    int soluciones = 0;
    char sol_cod[14];

    for(int k=0;k<ndud;k++){
        int pos = dudosos[k];
        if(pos < 1 || pos > 11) continue;
        if(cand_n[pos] <= 1) continue;

        char orig = cod[pos];
        char origp = 0;
        if(pos <= 6) origp = par[pos-1];

        for(int i=0;i<cand_n[pos];i++){
            int nd = cand_list[pos][i].d;
            char np = cand_list[pos][i].p;

            char nv = (char)('0'+nd);
            if(nv == orig && (pos>6 || np==origp)) continue;

            cod[pos] = nv;
            if(pos <= 6){
                par[pos-1] = np;
                int nfd = obtener_primer_digito(par);
                if(nfd < 0){
                    par[pos-1] = origp;
                    cod[pos] = orig;
                    continue;
                }
                cod[0] = (char)('0'+nfd);
            }

            if(calcular_dv(cod) == (cod[12]-'0')){
                soluciones++;
                strcpy(sol_cod, cod);
                if(soluciones > 1) break;
            }

            if(pos <= 6){
                par[pos-1] = origp;
                cod[0] = (char)('0'+fd);
            }
            cod[pos] = orig;
        }

        if(soluciones > 1) break;
    }

    if(soluciones == 1){
        strcpy(out, sol_cod);
        *dv_out = calcular_dv(out);
        return 0;
    }

    soluciones = 0;

    for(int a=0;a<ndud;a++){
        int p1 = dudosos[a];
        if(p1 < 1 || p1 > 11) continue;
        if(cand_n[p1] <= 1) continue;

        for(int b=a+1;b<ndud;b++){
            int p2 = dudosos[b];
            if(p2 < 1 || p2 > 11) continue;
            if(cand_n[p2] <= 1) continue;

            char o1 = cod[p1], o2 = cod[p2];
            char op1 = 0, op2 = 0;
            if(p1<=6) op1 = par[p1-1];
            if(p2<=6) op2 = par[p2-1];

            for(int i=0;i<cand_n[p1];i++){
                cod[p1] = (char)('0' + cand_list[p1][i].d);
                if(p1<=6) par[p1-1] = cand_list[p1][i].p;

                for(int j=0;j<cand_n[p2];j++){
                    cod[p2] = (char)('0' + cand_list[p2][j].d);
                    if(p2<=6) par[p2-1] = cand_list[p2][j].p;

                    int nfd = obtener_primer_digito(par);
                    if(nfd >= 0){
                        cod[0] = (char)('0'+nfd);
                        if(calcular_dv(cod) == (cod[12]-'0')){
                            soluciones++;
                            strcpy(sol_cod, cod);
                            if(soluciones > 1) break;
                        }
                    }

                    cod[p2] = o2;
                    if(p2<=6) par[p2-1] = op2;
                }

                if(soluciones > 1) break;

                cod[p1] = o1;
                if(p1<=6) par[p1-1] = op1;
            }

            cod[p1] = o1; cod[p2] = o2;
            if(p1<=6) par[p1-1] = op1;
            if(p2<=6) par[p2-1] = op2;
            cod[0] = (char)('0'+fd);

            if(soluciones > 1) break;
        }
        if(soluciones > 1) break;
    }

    if(soluciones == 1){
        strcpy(out, sol_cod);
        *dv_out = calcular_dv(out);
        return 0;
    }

    return 1;
}

int decodificar_ean13_desde_grises(const unsigned char *fila, int ancho, char *out, int *dv_out){
    if(out) out[0] = '\0';
    if(dv_out) *dv_out = -1;
    if(!fila || !out || !dv_out || ancho < 60) return 1;

 
    int th0 = umbral_histograma_simple(fila, ancho);
    int ths[5] = { th0-25, th0-10, th0, th0+10, th0+25 };
    for(int i=0;i<5;i++){
        if(ths[i] < 10) ths[i] = 10;
        if(ths[i] > 245) ths[i] = 245;
    }

    int *bin = (int*)malloc(sizeof(int)*ancho);
    if(!bin) return 1;

    int *tr = (int*)malloc(sizeof(int)*ancho);
    if(!tr){ free(bin); return 1; }

    for(int tcase=0;tcase<5;tcase++){
        int th = ths[tcase];

        for(int x=0;x<ancho;x++)
            bin[x] = (fila[x] < th) ? 1 : 0;

        int nt = 0;
        for(int x=1;x<ancho;x++){
            if(bin[x] != bin[x-1]) tr[nt++] = x;
        }
        if(nt < 20) continue;

        int amax = nt;
        if(amax > 200) amax = 200;

        for(int a=0; a<amax; a++){
            int start = tr[a];
            if(start < 0 || start >= ancho-1) continue;
            if(bin[start] != 1) continue; 

            int bmax = a + 260;
            if(bmax > nt) bmax = nt;

            for(int b=a+25; b<bmax; b++){
                int end = tr[b];
                if(end <= start + 60) continue;
                if(end >= ancho) break;

                int W = end - start;
                if(W < 80 || W > ancho) continue;

                char bits95[96];
                remuestrear_95_desde_intervalo(bin, start, end, bits95);

                if(!guardas_ok_bits95(bits95)) continue;

                {
                    int r = intentar_decodificar_bits95(bits95, EAN_L, EAN_G, EAN_R, EAN_PARITY, calcular_dv, obtener_primer_digito, candidatos, out, dv_out);

                    if(r == 0){
                        free(bin);
                        free(tr);
                        return 0;
                    }
                }
            }
        }
    }

    free(bin);
    free(tr);
    return 1;
}

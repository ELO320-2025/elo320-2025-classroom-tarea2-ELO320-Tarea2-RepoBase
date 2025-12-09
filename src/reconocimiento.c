#include "reconocimiento.h"

static const char *EAN_L_PATTERNS[10] = {
    "0001101","0011001","0010011","0111101","0100011",
    "0110001","0101111","0111011","0110111","0001011"
};

static const char *EAN_G_PATTERNS[10] = {
    "0100111","0110011","0011011","0100001","0011101",
    "0111001","0000101","0010001","0001001","0010111"
};

static const char *EAN_R_PATTERNS[10] = {
    "1110010","1100110","1101100","1000010","1011100",
    "1001110","1010000","1000100","1001000","1110100"
};

static const char *EAN_FIRST_DIGIT_PARITY[10] = {
    "LLLLLL","LLGLGG","LLGGLG","LLGGGL","LGLLGG",
    "LGGLLG","LGGGLL","LGLGLG","LGLGGL","LGGLGL"
};

static int decodificar_digito_izquierdo(const char *p, char *par) {
    int d;
    for (d = 0; d < 10; d++)
        if (strcmp(p, EAN_L_PATTERNS[d]) == 0) { *par = 'L'; return d; }
    for (d = 0; d < 10; d++)
        if (strcmp(p, EAN_G_PATTERNS[d]) == 0) { *par = 'G'; return d; }
    return -1;
}

static int decodificar_digito_derecho(const char *p) {
    int d;
    for (d = 0; d < 10; d++)
        if (strcmp(p, EAN_R_PATTERNS[d]) == 0) return d;
    return -1;
}

static int obtener_primer_digito(const char *par) {
    int d;
    for (d = 0; d < 10; d++)
        if (strcmp(par, EAN_FIRST_DIGIT_PARITY[d]) == 0) return d;
    return -1;
}

static int calcular_dv(const char *cod12) {
    int s1 = 0, s2 = 0;
    for (int i = 0; i < 12; i++) {
        int v = cod12[i] - '0';
        if (i % 2 == 0) s1 += v;
        else s2 += v;
    }
    int t = s1 + 3 * s2;
    int r = t % 10;
    return (r == 0) ? 0 : 10 - r;
}

int decodificar_ean13_desde_grises(const unsigned char *fila, int ancho,char *out,int *dv_out) {
    if (!fila || !out || !dv_out || ancho <= 0)
        return 1;

    int histo[256];
    for (int i = 0; i < 256; i++) histo[i] = 0;
    for (int x = 0; x < ancho; x++) histo[fila[x]]++;

    int pico = 0;
    for (int i = 1; i < 256; i++)
        if (histo[i] > histo[pico]) pico = i;

    int umbral = pico - 40;
    if (umbral < 20) umbral = 20;
    if (umbral > 200) umbral = 200;

    int *bin = malloc(sizeof(int) * ancho);
    if (!bin) return 1;
    for (int x = 0; x < ancho; x++)
        bin[x] = (fila[x] < umbral) ? 1 : 0;

    int *run_len = malloc(sizeof(int) * ancho);
    int *run_bit = malloc(sizeof(int) * ancho);
    if (!run_len || !run_bit) {
        free(bin);
        free(run_len);
        free(run_bit);
        return 1;
    }

    int run_count = 0;
    int cur_bit = bin[0];
    int cur_len = 1;

    for (int x = 1; x < ancho; x++) {
        if (bin[x] == cur_bit) {
            cur_len++;
        } else {
            run_len[run_count] = cur_len;
            run_bit[run_count] = cur_bit;
            run_count++;
            cur_bit = bin[x];
            cur_len = 1;
        }
    }
    run_len[run_count] = cur_len;
    run_bit[run_count] = cur_bit;
    run_count++;

    if (run_count < 10) {
        free(bin);
        free(run_len);
        free(run_bit);
        return 1;
    }

    int min_len = run_len[0];
    for (int r = 1; r < run_count; r++)
        if (run_len[r] < min_len) min_len = run_len[r];
    if (min_len <= 0) {
        free(bin);
        free(run_len);
        free(run_bit);
        return 1;
    }

    double base = (double)min_len;

    int *run_mod = malloc(sizeof(int) * run_count);
    if (!run_mod) {
        free(bin);
        free(run_len);
        free(run_bit);
        return 1;
    }

    int total_mod = 0;
    for (int r = 0; r < run_count; r++) {
        double m = run_len[r] / base;
        int mi = (int)(m + 0.5);
        if (mi < 1) mi = 1;
        run_mod[r] = mi;
        total_mod += mi;
    }

    if (total_mod < 95 || total_mod > 2000) {
        free(bin);
        free(run_len);
        free(run_bit);
        free(run_mod);
        return 1;
    }

    int *mods = malloc(sizeof(int) * total_mod);
    if (!mods) {
        free(bin);
        free(run_len);
        free(run_bit);
        free(run_mod);
        return 1;
    }

    int mpos = 0;
    for (int r = 0; r < run_count; r++) {
        for (int k = 0; k < run_mod[r]; k++) {
            if (mpos >= total_mod) break;
            mods[mpos++] = run_bit[r];
        }
        if (mpos >= total_mod) break;
    }

    if (mpos < 95) {
        free(bin);
        free(run_len);
        free(run_bit);
        free(run_mod);
        free(mods);
        return 1;
    }

    int start = -1;
    for (int s = 0; s + 94 < mpos; s++) {
        if (!(mods[s] == 1 && mods[s+1] == 0 && mods[s+2] == 1))
            continue;
        if (!(mods[s+45] == 0 && mods[s+46] == 1 && mods[s+47] == 0 &&
              mods[s+48] == 1 && mods[s+49] == 0))
            continue;
        if (!(mods[s+92] == 1 && mods[s+93] == 0 && mods[s+94] == 1))
            continue;
        start = s;
        break;
    }

    if (start < 0) {
        free(bin);
        free(run_len);
        free(run_bit);
        free(run_mod);
        free(mods);
        return 1;
    }

    char bits[96];
    for (int i = 0; i < 95; i++)
        bits[i] = mods[start + i] ? '1' : '0';

    free(bin);
    free(run_len);
    free(run_bit);
    free(run_mod);
    free(mods);

    if (!(bits[0]=='1' && bits[1]=='0' && bits[2]=='1')) return 1;
    if (!(bits[45]=='0' && bits[46]=='1' && bits[47]=='0' &&
          bits[48]=='1' && bits[49]=='0')) return 1;
    if (!(bits[92]=='1' && bits[93]=='0' && bits[94]=='1')) return 1;

    char cod[14];
    char par[7];
    par[6] = '\0';

    for (int d = 0; d < 6; d++) {
        int offs = 3 + 7*d;
        char p[8];
        for (int t = 0; t < 7; t++)
            p[t] = bits[offs+t];
        p[7] = '\0';

        char pin;
        int val = decodificar_digito_izquierdo(p, &pin);
        if (val < 0) return 1;
        cod[1+d] = '0' + val;
        par[d] = pin;
    }

    for (int d = 0; d < 6; d++) {
        int offs = 50 + 7*d;
        char p[8];
        for (int t = 0; t < 7; t++)
            p[t] = bits[offs+t];
        p[7] = '\0';

        int val = decodificar_digito_derecho(p);
        if (val < 0) return 1;
        cod[7+d] = '0' + val;
    }

    int fd = obtener_primer_digito(par);
    if (fd < 0) return 1;
    cod[0] = '0' + fd;

    cod[13] = '\0';

    int dv_calc = calcular_dv(cod);
    int dv_img = cod[12] - '0';

    *dv_out = dv_calc;
    strcpy(out, cod);

    if (dv_calc != dv_img) return 2;

    return 0;
}
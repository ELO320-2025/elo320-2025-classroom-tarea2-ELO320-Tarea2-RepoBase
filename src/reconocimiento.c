#include "reconocimiento.h"

static int tablaImpar[10][7] = {
{0,0,0,1,1,0,1},
{0,0,1,1,0,0,1},
{0,0,1,0,0,1,1},
{0,1,1,1,1,0,1},
{0,1,0,0,0,1,1},
{0,1,1,0,0,0,1},
{0,1,0,1,1,1,1},
{0,1,1,1,0,1,1},
{0,1,1,0,1,1,1},
{0,0,0,1,0,1,1}
};

static int tablaPar[10][7] = {
{0,1,0,0,1,1,1},
{0,1,1,0,0,1,1},
{0,0,1,1,0,1,1},
{0,1,0,0,0,0,1},
{0,0,1,1,1,0,1},
{0,1,1,1,0,0,1},
{0,0,0,0,1,0,1},
{0,0,1,0,0,0,1},
{0,0,0,1,0,0,1},
{0,1,0,1,1,1,1}
};

static int tablaDer[10][7] = {
{1,1,1,0,0,1,0},
{1,1,0,0,1,1,0},
{1,1,0,1,1,0,0},
{1,0,0,0,0,0,1},
{1,0,1,1,1,0,0},
{1,0,0,1,1,1,0},
{1,0,1,0,0,0,0},
{1,0,0,0,1,0,0},
{1,0,0,1,0,0,0},
{1,1,1,0,1,0,0}
};

static int miRound(double x) {
    if (x >= 0) return (int)(x + 0.5);
    return (int)(x - 0.5);
}

SecuenciaBarras obtenerSecuenciaBarras(int *fila, int largo) {
    SecuenciaBarras s;
    s.anchos = malloc(sizeof(int) * largo);
    s.largo = 0;
    if (largo <= 0) return s;
    int actual = fila[0];
    int conteo = 1;
    int i;
    for (i = 1; i < largo; i++) {
        if (fila[i] == actual) conteo++;
        else {
            s.anchos[s.largo++] = conteo;
            actual = fila[i];
            conteo = 1;
        }
    }
    s.anchos[s.largo++] = conteo;
    return s;
}

SecuenciaBarras normalizarSecuencia(SecuenciaBarras s) {
    SecuenciaBarras n;
    n.largo = s.largo;
    n.anchos = malloc(sizeof(int) * s.largo);
    int min = s.anchos[0];
    int i;
    for (i = 1; i < s.largo; i++) {
        if (s.anchos[i] < min) min = s.anchos[i];
    }
    if (min <= 0) min = 1;
    for (i = 0; i < s.largo; i++) {
        int v = miRound((double)s.anchos[i] / (double)min);
        if (v < 1) v = 1;
        n.anchos[i] = v;
    }
    return n;
}

int detectarInicio(SecuenciaBarras s) {
    int i;
    for (i = 0; i < s.largo - 2; i++) {
        if (s.anchos[i] == 1 && s.anchos[i+1] == 1 && s.anchos[i+2] == 1) return i;
    }
    return -1;
}

int detectarCentro(SecuenciaBarras s, int inicio) {
    int i;
    for (i = inicio + 3; i < s.largo - 4; i++) {
        if (s.anchos[i] == 1 && s.anchos[i+1] == 1 && s.anchos[i+2] == 1 && s.anchos[i+3] == 1 && s.anchos[i+4] == 1)
            return i;
    }
    return -1;
}

static void normalizarDigito(int *src, int *dst) {
    int sum = src[0] + src[1] + src[2] + src[3];
    if (sum == 0) sum = 1;
    dst[0] = miRound((double)src[0] * 7.0 / (double)sum);
    dst[1] = miRound((double)src[1] * 7.0 / (double)sum);
    dst[2] = miRound((double)src[2] * 7.0 / (double)sum);
    dst[3] = miRound((double)src[3] * 7.0 / (double)sum);
    if (dst[0] < 1) dst[0] = 1;
    if (dst[1] < 1) dst[1] = 1;
    if (dst[2] < 1) dst[2] = 1;
    if (dst[3] < 1) dst[3] = 1;
}

static void expandirBitsIzq(int *a, int *bits) {
    int color = 0;
    int k = 0;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < a[i]; j++) {
            if (k < 7) bits[k++] = color;
        }
        color = 1 - color;
    }
    while (k < 7) {
        bits[k] = bits[k-1];
        k++;
    }
}

static void expandirBitsDer(int *a, int *bits) {
    int color = 1;
    int k = 0;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < a[i]; j++) {
            if (k < 7) bits[k++] = color;
        }
        color = 1 - color;
    }
    while (k < 7) {
        bits[k] = bits[k-1];
        k++;
    }
}

static int decodificar7(int *bits, int modo) {
    int i, j, ok;
    if (modo == 0) {
        for (i = 0; i < 10; i++) {
            ok = 1;
            for (j = 0; j < 7; j++) if (bits[j] != tablaImpar[i][j]) ok = 0;
            if (ok) return i;
        }
    }
    if (modo == 1) {
        for (i = 0; i < 10; i++) {
            ok = 1;
            for (j = 0; j < 7; j++) if (bits[j] != tablaPar[i][j]) ok = 0;
            if (ok) return i;
        }
    }
    for (i = 0; i < 10; i++) {
        ok = 1;
        for (j = 0; j < 7; j++) if (bits[j] != tablaDer[i][j]) ok = 0;
        if (ok) return i;
    }
    return -1;
}

CodigoEAN13 decodificarEAN13(SecuenciaBarras s, int inicio, int centro) {
    CodigoEAN13 c;
    int i;
    for (i = 0; i < 13; i++) c.digitos[i] = -1;
    c.valido = 0;
    if (s.largo < 3 + 24 + 5 + 24 + 3) return c;

    int idxFinal = -1;
    for (i = s.largo - 3; i >= 0; i--) {
        if (s.anchos[i] == 1 && s.anchos[i+1] == 1 && s.anchos[i+2] == 1) {
            idxFinal = i;
            break;
        }
    }
    if (idxFinal < 0) return c;

    int idxRightStart = idxFinal - 24;
    if (idxRightStart - 5 - 24 - 3 < 0) return c;

    int idxCenterStart = idxRightStart - 5;
    int idxLeftStart = idxCenterStart - 24;

    int bits[7];
    int raw[4];
    int a4[4];

    int tempRight[6];
    int idx = idxFinal - 1;
    int k;
    for (k = 0; k < 6; k++) {
        if (idx - 3 < 0) return c;
        raw[3] = s.anchos[idx];
        raw[2] = s.anchos[idx-1];
        raw[1] = s.anchos[idx-2];
        raw[0] = s.anchos[idx-3];
        normalizarDigito(raw, a4);
        expandirBitsDer(a4, bits);
        tempRight[k] = decodificar7(bits, 2);
        idx -= 4;
    }
    for (k = 0; k < 6; k++) c.digitos[12-k] = tempRight[k];

    int tempLeft[6];
    idx = idxLeftStart;
    for (k = 0; k < 6; k++) {
        raw[0] = s.anchos[idx];
        raw[1] = s.anchos[idx+1];
        raw[2] = s.anchos[idx+2];
        raw[3] = s.anchos[idx+3];
        normalizarDigito(raw, a4);
        expandirBitsIzq(a4, bits);
        tempLeft[k] = decodificar7(bits, 0);
        idx += 4;
    }
    for (k = 0; k < 6; k++) c.digitos[1+k] = tempLeft[k];

    c.digitos[0] = 0;

    int suma = 0;
    for (i = 0; i < 12; i++) {
        if ((i % 2) == 0) suma += c.digitos[i];
        else suma += 3 * c.digitos[i];
    }
    c.digitos[12] = (10 - (suma % 10)) % 10;
    c.valido = 1;
    return c;
}

int validarControl(CodigoEAN13 c) {
    int suma = 0;
    int i;
    for (i = 0; i < 12; i++) {
        if ((i % 2) == 0) suma += c.digitos[i];
        else suma += 3 * c.digitos[i];
    }
    int ctrl = (10 - (suma % 10)) % 10;
    return ctrl == c.digitos[12];
}
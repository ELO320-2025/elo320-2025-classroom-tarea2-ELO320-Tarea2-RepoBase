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

static int mejorGuarda(SecuenciaBarras s) {
    int i;
    double best = 1e18;
    int pos = -1;
    for (i = 0; i < s.largo - 2; i++) {
        double a = s.anchos[i];
        double b = s.anchos[i+1];
        double c = s.anchos[i+2];
        if (a <= 0 || b <= 0 || c <= 0) continue;
        double d1 = fabs(a - b);
        double d2 = fabs(b - c);
        double d3 = fabs(a - c);
        double score = d1 + d2 + d3;
        if (score < best) {
            best = score;
            pos = i;
        }
    }
    return pos;
}

SecuenciaBarras normalizarSecuencia(SecuenciaBarras s) {
    SecuenciaBarras n;
    n.largo = s.largo;
    n.anchos = malloc(sizeof(int) * s.largo);

    int g = mejorGuarda(s);
    if (g < 0) {
        int i;
        for (i = 0; i < s.largo; i++) n.anchos[i] = 1;
        return n;
    }

    double M = (s.anchos[g] + s.anchos[g+1] + s.anchos[g+2]) / 3.0;
    if (M < 1) M = 1;

    int i;
    for (i = 0; i < s.largo; i++) {
        int v = miRound(s.anchos[i] / M);
        if (v < 1) v = 1;
        n.anchos[i] = v;
    }
    return n;
}

int detectarInicio(SecuenciaBarras s) {
    int i;
    for (i = 0; i < s.largo - 2; i++) {
        if (s.anchos[i] == 1 && s.anchos[i+1] == 1 && s.anchos[i+2] == 1)
            return i;
    }
    return -1;
}

int detectarCentro(SecuenciaBarras s, int ini) {
    int i;
    for (i = ini + 3; i < s.largo - 4; i++) {
        if (s.anchos[i] == 1 && s.anchos[i+1] == 1 && s.anchos[i+2] == 1 &&
            s.anchos[i+3] == 1 && s.anchos[i+4] == 1)
            return i;
    }
    return -1;
}

static void expandirBits(int *a, int colorInicio, int *bits) {
    int color = colorInicio;
    int k = 0;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < a[i]; j++) {
            if (k < 7) bits[k++] = color;
        }
        color = 1 - color;
    }
    while (k < 7) bits[k] = bits[k-1], k++;
}

static int decImpar(int *bits) {
    int i, j, ok;
    for (i = 0; i < 10; i++) {
        ok = 1;
        for (j = 0; j < 7; j++) if (bits[j] != tablaImpar[i][j]) ok = 0;
        if (ok) return i;
    }
    return -1;
}

static int decPar(int *bits) {
    int i, j, ok;
    for (i = 0; i < 10; i++) {
        ok = 1;
        for (j = 0; j < 7; j++) if (bits[j] != tablaPar[i][j]) ok = 0;
        if (ok) return i;
    }
    return -1;
}

static int decDer(int *bits) {
    int i, j, ok;
    for (i = 0; i < 10; i++) {
        ok = 1;
        for (j = 0; j < 7; j++) if (bits[j] != tablaDer[i][j]) ok = 0;
        if (ok) return i;
    }
    return -1;
}

static int deducirPrimer(int *p) {
    int tabla[10][6] = {
        {0,0,0,0,0,0},
        {0,0,1,0,1,1},
        {0,0,1,1,0,1},
        {0,0,1,1,1,0},
        {0,1,0,0,1,1},
        {0,1,1,0,0,1},
        {0,1,1,1,0,0},
        {0,1,0,1,0,1},
        {0,1,0,1,1,0},
        {0,1,1,0,1,0}
    };
    int d,i;
    for (d = 0; d < 10; d++) {
        int ok = 1;
        for (i = 0; i < 6; i++) if (tabla[d][i] != p[i]) ok = 0;
        if (ok) return d;
    }
    return 0;
}

CodigoEAN13 decodificarEAN13(SecuenciaBarras s, int ini, int cen) {
    CodigoEAN13 c;
    int i;
    for (i = 0; i < 13; i++) c.digitos[i] = 0;
    c.valido = 0;

    if (ini < 0) ini = detectarInicio(s);
    if (cen < 0) cen = detectarCentro(s, ini);
    if (ini < 0 || cen < 0) return c;

    int baseIzq = ini + 3;
    int baseDer = cen + 5;

    int bits[7];
    int a4[4];
    int pz[6];
    int izq[6];
    int der[6];

    int pos = baseIzq;
    for (i = 0; i < 6; i++) {
        a4[0] = s.anchos[pos];
        a4[1] = s.anchos[pos+1];
        a4[2] = s.anchos[pos+2];
        a4[3] = s.anchos[pos+3];
        expandirBits(a4, 0, bits);
        int dL = decImpar(bits);
        int dG = decPar(bits);
        if (dL >= 0) {
            izq[i] = dL;
            pz[i] = 0;
        } else if (dG >= 0) {
            izq[i] = dG;
            pz[i] = 1;
        } else return c;
        pos += 4;
    }

    pos = baseDer;
    for (i = 0; i < 6; i++) {
        a4[0] = s.anchos[pos];
        a4[1] = s.anchos[pos+1];
        a4[2] = s.anchos[pos+2];
        a4[3] = s.anchos[pos+3];
        expandirBits(a4, 1, bits);
        int d = decDer(bits);
        if (d < 0) return c;
        der[i] = d;
        pos += 4;
    }

    c.digitos[0] = deducirPrimer(pz);
    for (i = 0; i < 6; i++) c.digitos[1+i] = izq[i];
    for (i = 0; i < 6; i++) c.digitos[7+i] = der[i];

    int sum = 0;
    for (i = 0; i < 12; i++) {
        if ((i % 2) == 0) sum += c.digitos[i];
        else sum += 3 * c.digitos[i];
    }
    c.digitos[12] = (10 - (sum % 10)) % 10;
    c.valido = 1;
    return c;
}

int validarControl(CodigoEAN13 c) {
    int sum = 0;
    int i;
    for (i = 0; i < 12; i++) {
        if ((i % 2) == 0) sum += c.digitos[i];
        else sum += 3 * c.digitos[i];
    }
    int ctrl = (10 - (sum % 10)) % 10;
    return ctrl == c.digitos[12];
}
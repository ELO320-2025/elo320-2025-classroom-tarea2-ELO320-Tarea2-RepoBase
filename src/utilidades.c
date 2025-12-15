#include "utilidades.h"

int ean13_validar(const char *cod) {
    if (!cod) return 0;
    if (strlen(cod) != 13) return 0;

    int i;
    for (i = 0; i < 13; i++) {
        if (cod[i] < '0' || cod[i] > '9')
            return 0;
    }

    int s1 = 0;
    int s2 = 0;

    for (i = 0; i < 12; i++) {
        int v = cod[i] - '0';
        if (i % 2 == 0)
            s1 += v;
        else
            s2 += v;
    }

    int total = s1 + 3 * s2;
    int r = total % 10;
    int dv = (r == 0) ? 0 : 10 - r;

    return dv == (cod[12] - '0');
}

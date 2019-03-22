#include <math.h>
#include <complex.h>
#include "dft.h"

void
dft(double *samples, size_t count, double complex *result)
{
    if (result == NULL) {
        return;
    }

    for (int k = 0; k < count; k++) {
        double a = 2.0 * M_PI * k / count;
        result[k] = CMPLX(0, 0);

        /* Process only the left or right side channel at once */
        for (int n = 0; n < count; n++) {
            double sample = samples[n];
            double real = cos(a * n);
            double imag = sin(a * n);
            result[k] += sample * (real - imag * I);
        }
    }
}


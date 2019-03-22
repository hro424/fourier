#ifndef FOURIER_DFT_H
#define FOURIER_DFT_H

#include <complex.h>
#include "wave.h"

/**
 * Discrete Fourier Transformation
 *
 * @param samples   the signal samples.
 * @param count     the number of samples to be processed.
 * @param result    the result of DFT.
 */
void dft(double *samples, size_t count, double complex *result);

#endif /* FOURIER_DFT_H */

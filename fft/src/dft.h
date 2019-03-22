#ifndef FOURIER_DFT_H
#define FOURIER_DFT_H

#include <complex.h>
#include "wave.h"

/**
 * Discrete Fourier Transformation
 *
 * @param handle    the handler of the wave file.
 * @param buf       the wave data.
 * @param result    the result of DFT.
 */
void dft_wave(wave_handle_t *handle, wave_buffer_t *buf, double complex *result);

/**
 * Discrete Fourier Transformation
 *
 * @param channels  the number of channels.
 * @param samples   the signal samples.
 * @param count     the number of samples to be processed.
 * @param result    the result of DFT.
 */
void dft(size_t channels, double *samples, size_t count, double complex *result);

#endif /* FOURIER_DFT_H */

#include "dft.h"

void
dft_wave(wave_handle_t *handle, wave_buffer_t *buf, double complex *result)
{
    dft(handle->num_channels, buf->buffer, handle->sample_rate, result);
}

void
dft(size_t channels, double *samples, size_t count, double complex *result)
{
    if (result == NULL) {
        //TODO: error report
        return;
    }

    for (int k = 0; k < count; k++) {
        double a = 2.0 * M_PI * k / count;

        for (int c = 0; c < channels; c++) {
            int index = k * channels + c;
            result[index] = CMPLX(0, 0);

            /* Process only the left or right side channel at once */
            for (int n = 0; n < count; n++) {
                double sample = samples[n * channels + c];
                double real = cos(a * n);
                double imag = sin(a * n);
                result[index] += sample * (real - imag * I);
            }
        }
    }
}


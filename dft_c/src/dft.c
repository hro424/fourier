#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include "wave.h"


/**
 * Discrete Fourier Transformation
 *
 * @param sample_rate   sampling rate of the wave data.
 * @param buf           the wave data.
 * @param length        the length to be processed.
 * @param result_real   the result of DFT.
 * @param result_imag   the result of DFT.
 */
void
dft(wave_handle_t *handle, wave_buffer_t *buf, size_t length,
        double *result_real, double *result_imag)
{
#if 0
    int ch = handle->num_channels;
    int sample_count = ch * handle->sample_rate;

    for (int k = 0; k < sample_count; k++) {
        int kk = k / ch;
        double a = 2.0 * M_PI * kk / handle->sample_rate;

        if (result_real != NULL) {
            result_real[k] = 0.0;
        }

        if (result_imag != NULL) {
            result_imag[k] = 0.0;
        }

        /* Process only the left or right side channel at once */
        for (int n = k & 1; n < sample_count; n += ch) {
            int nn = n / ch;
            if (result_real != NULL) {
                double real = cos(a * nn);
                result_real[k] += real * buf->buffer[n];
            }

            if (result_imag != NULL) {
                double imag = sin(a * nn);
                result_imag[k] += imag * buf->buffer[n];
            }
        }

        if (result_real != NULL) {
            result_real[k] = fabs(result_real[k]);
        }

        if (result_imag != NULL) {
            result_imag[k] = fabs(result_imag[k]);
        }
    }
#else
    uint32_t sample_rate = handle->sample_rate;

    for (int k = 0; k < sample_rate; k++) {
        double a = 2.0 * M_PI * k / sample_rate;

        if (result_real != NULL) {
            result_real[k * 2] = 0.0;
        }

        if (result_imag != NULL) {
            result_imag[k * 2] = 0.0;
        }

        for (int n = 0; n < sample_rate; n++) {
            if (result_real != NULL) {
                double real = cos(a * n);
                result_real[k * 2] += real * buf->buffer[n * 2];
            }

            if (result_imag != NULL) {
                double imag = sin(a * n);
                result_imag[k * 2] += imag * buf->buffer[n * 2];
            }
        }

        if (result_real != NULL) {
            result_real[k * 2] = fabs(result_real[k * 2]);
        }

        if (result_imag != NULL) {
            result_imag[k * 2] = fabs(result_imag[k * 2]);
        }
    }
#endif
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    wave_handle_t *handle;
    wave_buffer_t *buffer;

    handle = wave_open(argv[1], O_RDONLY);
    if (handle == NULL) {
        return EXIT_FAILURE;
    }

    printf("# length %u\n", handle->length);
    printf("# num_channels %u\n", handle->num_channels);
    printf("# sample_rate %u\n", handle->sample_rate);
    printf("# byte_rate %u\n", handle->byte_rate);
    printf("# block_size %u\n", handle->block_size);
    printf("# bits_per_sample %u\n", handle->bits_per_sample);

    buffer = wave_alloc_buffer(handle, 1);
    ssize_t length = wave_read(handle, buffer);
    printf("# %zd samples read.\n", length);

    // This trick is necessary... Why?
    for (int i = length / 2; i < length; i++) {
        buffer->buffer[i] = 0.0;
    }

    double *result = malloc(sizeof(double) * length);
    dft(handle, buffer, length, result, NULL);

    /* Output only the left channel */
    for (int i = 0; i < length / 4; i++) {
        printf("%d %f\n", i, result[i * 2]);
    }

    free(result);
    wave_free_buffer(buffer);
    wave_close(handle);

    return EXIT_SUCCESS;
}

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
 * @param length        the length of the data in bytes.
 */
void
dft(wave_handle_t *handle, wave_buffer_t *buf, size_t length,
        double *result_real, double *result_imag)
{
    for (int k = 0; k < handle->sample_rate * handle->num_channels; k++) {
        double a = 2.0 * M_PI * k / handle->sample_rate;

        if (result_real != NULL) {
            result_real[k] = 0;
        }

        if (result_imag != NULL) {
            result_imag[k] = 0;
        }

        int n = k & 1;
        for (; n < handle->sample_rate; n++) {
            if (result_real != NULL) {
                double real = cos(a * n);
                result_real[k] += real * buf->buffer[n];
            }

            if (result_imag != NULL) {
                double imag = sin(a * n);
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
    printf("# Buffer allocated: %lu bytes\n", buffer->length);

    ssize_t length = wave_read(handle, buffer);

    double *result = malloc(sizeof(double) * length);
    dft(handle->sample_rate, buffer, length, result, NULL);

    for (int i = 0; i < length / 2; i++) {
        printf("%d %f\n", i, result[i]);
    }

    free(result);
    wave_free_buffer(buffer);
    wave_close(handle);

    return EXIT_SUCCESS;
}

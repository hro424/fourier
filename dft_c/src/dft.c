#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include "wave.h"


void
dft(uint32_t sample_rate, wave_buffer_t *buf, size_t length)
{
    int16_t *block = (int16_t *)buf->buffer;
    double *sample_real;
    double *sample_imag;
    double *result_real;
    double *result_imag;

    sample_real = malloc(sizeof(double) * sample_rate);
    sample_imag = malloc(sizeof(double) * sample_rate);
    result_real = malloc(sizeof(double) * sample_rate);
    result_imag = malloc(sizeof(double) * sample_rate);

    printf("# Analyze %u samples.\n", sample_rate);

    for (int n = 0; n < sample_rate; n++) {
        sample_real[n] = (double)block[n * 2] / 32768.0;
        sample_imag[n] = 0.0;
    }

    for (int k = 0; k < sample_rate; k++) {
        double a = 2.0 * M_PI * k / sample_rate;
        result_real[k] = 0;
        result_imag[k] = 0;
        for (int n = 0; n < sample_rate; n++) {
            double real = cos(a * n);
            //double imag = -sin(a * n);
            result_real[k] += real * sample_real[n];
            result_imag[k] += real * sample_imag[n];
        }
    }

    for (int k = 0; k < sample_rate / 2; k++) {
        printf("%d %f\n", k, fabs(result_real[k]));
    }

    free(sample_real);
    free(sample_imag);
    free(result_real);
    free(result_imag);
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

    buffer = wave_alloc_buffer(handle, 2);
    printf("# Buffer allocated: %lu bytes\n", buffer->length);

    ssize_t length = wave_read(handle, buffer);
    printf("# %ld bytes read\n", length);
    /*
    int16_t *ptr = (int16_t *)buffer->buffer;
    for (int i = 0; i < 100; i++) {
        printf("%d\n", ptr[i]);
    }
    */

    dft(handle->sample_rate, buffer, length / handle->block_size);

    wave_free_buffer(buffer);
    wave_close(handle);

    return EXIT_SUCCESS;
}

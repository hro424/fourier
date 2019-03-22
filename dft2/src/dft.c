#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <complex.h>
#include "dft.h"
#include "wave.h"


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

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    wave_handle_t *handle;
    wave_read_buffer_t *rbuf;

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

    rbuf = wave_alloc_read_buffer(handle, 1);
    printf("rbuf len: %zu\n", rbuf->length);
    ssize_t length = wave_rawread(handle, rbuf);
    printf("length %zd\n", length);

    size_t len = length / wave_bsize(handle); /* length for a single channel */
    double complex *result = malloc(sizeof(double complex) * len);
    double *tmp = malloc(sizeof(double) * len);

    wave_single_channel(handle, rbuf, tmp, len, 0);

    dft(tmp, wave_sr(handle), result);

    /* Output only the left channel */
    for (int i = 0; i < len / 2; i++) {
        printf("%d %f %f 0 0\n", i, cabs(result[i]), carg(result[i]));
    }

    free(tmp);
    free(result);
    wave_free_read_buffer(rbuf);
    wave_close(handle);

    return EXIT_SUCCESS;
}

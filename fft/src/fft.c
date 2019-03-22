#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <complex.h>
#include "wave.h"
#include "dft.h"

static size_t*
create_index_table(size_t exp)
{
    size_t *table = malloc(sizeof(size_t) * (1 << exp));

    if (table != NULL) {
        for (size_t i = 0; i < (1 << exp); i++) {
            size_t res = 0;
            for (size_t b = 0; b < exp; b++) {
                res |= ((i >> b) & 1) << (exp - b - 1);
            }
            table[i] = res;
        }
    }

    return table;
}

static void
destroy_index_table(size_t *table)
{
    free(table);
}

static void
copy_and_sort(double *dest, size_t exp, double *src, size_t count)
{
    size_t *itable = create_index_table(exp);
    if (itable == NULL) {
        //TODO: Error report
        printf("ERROR\n");
        return;
    }

    if ((1 << exp) < count) {
        count = 1 << exp;
    }

    for (int i = 0; i < count; i++) {
        dest[itable[i]] = src[i];
//printf("%d->%zu %f\n", i, itable[i], src[i]);
    }

    destroy_index_table(itable);
}

static void
fold(size_t exp, double complex *input, double complex *output, size_t length)
{
    int N = 1 << exp;
    double a = 2.0 * M_PI / (double)N;
    for (int i = 0; i < length / N; i++) {
        for (int j = 0; j < N / 2; j++) {
            double real = cos(a * j);
            double imag = sin(a * j);
            int m = i * N + j;
            int n = m + N / 2;
            double complex delta = CMPLX(real, imag) * input[n];
            output[m] = input[m] + delta;
            output[n] = input[m] - delta;
        }
    }
}

/**
 * Fold the result of DFT.  Uses the input array in a corruptive way.
 */
static inline void
solve(double complex *input, double complex *output, size_t exp)
{
    for (size_t i = 2; i < exp; i++) {
        fold(i, input, output, 1 << exp);
/*
printf("%zu:\n", i);
for (int j = 0; j < 16; j++) {
    printf("%d %f  %f\n", j, creal(input[j]), creal(output[j]));
}
*/
        memcpy(input, output, sizeof(double complex) * (1 << exp));
    }
}

static inline size_t
to_exp(size_t count)
{
    size_t exp;
    for (exp = 0; count > 0; count >>= 1, exp++) ;
    return exp;
}

/**
 * Fast Fourier Transform.
 *
 * @param sample    the signal samples.
 * @param count     the number of samples.
 * @param result    the result of transform.
 */
void
fft(double *samples, size_t count, double complex *result)
{
    /*
     * Expand and align the buffer size to power of 2, which is the
     * assumption of FFT.
     */
    size_t exp = to_exp(count - 1);
    size_t extended = 1 << exp;
    double *buf = calloc(extended, sizeof(double));
    if (buf == NULL) {
        //TODO: Error report
        printf("ERROR\n");
        return;
    }

    copy_and_sort(buf, exp, samples, count);

    /* DFT (N = 2) */
    const int N = 2;
    double complex *dft_result = malloc(sizeof(double complex) * extended);
    if (dft_result == NULL) {
        //TODO: Error report
        printf("ERROR\n");
        return;
    }

    for (int i = 0; i < extended; i += N) {
        //dft(&buf[i], N, &dft_result[i]);
        dft2(&buf[i], &dft_result[i]);
    }

    free(buf);

    /* Fold */
    solve(dft_result, result, exp);

    free(dft_result);
}

static void
dump(wave_handle_t *h)
{
    printf("# length %u\n", h->length);
    printf("# num_channels %u\n", h->num_channels);
    printf("# sample_rate %u\n", h->sample_rate);
    printf("# byte_rate %u\n", h->byte_rate);
    printf("# block_size %u\n", h->block_size);
    printf("# bits_per_sample %u\n", h->bits_per_sample);
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    wave_handle_t *handle;
    handle = wave_open(argv[1], O_RDONLY);
    if (handle == NULL) {
        return EXIT_FAILURE;
    }

    dump(handle);

    wave_read_buffer_t *rbuf = wave_alloc_read_buffer(handle, 1);

    ssize_t length = wave_rawread(handle, rbuf);
    printf("# %zd samples read.\n", length);

    size_t len = length / wave_bsize(handle);
    double complex *result = calloc(len, sizeof(double complex));
    double *tmp = calloc(len, sizeof(double));
    printf("# %zu samples to be processed.\n", len);

    wave_single_channel(handle, rbuf, tmp, len, 0);

    fft(tmp, len, result);

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

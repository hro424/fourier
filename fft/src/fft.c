#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <complex.h>
#include "wave.h"

static inline size_t
to_exp(size_t count)
{
    size_t exp;
    for (exp = 0; count > 0; count >>= 1, exp++) ;
    return exp;
}

static inline size_t*
create_index_table(size_t exp)
{
    size_t length = 1 << exp;
    size_t *table = malloc(sizeof(size_t) * length);

    if (table != NULL) {
        for (size_t i = 0; i < length; i++) {
            size_t res = 0;
            for (size_t b = 0; b < exp; b++) {
                res |= ((i >> b) & 1) << (exp - b - 1);
            }
            table[i] = res;
        }
    }

    return table;
}

static inline void
destroy_index_table(size_t *table)
{
    free(table);
}

static int
copy_and_sort(double complex *dest, size_t exp, double *src, size_t count)
{
    size_t *itable = create_index_table(exp);
    if (itable == NULL) {
        return -1;
    }

    if ((1 << exp) < count) {
        count = 1 << exp;
    }

    for (int i = 0; i < count; i++) {
        dest[itable[i]] = src[i];
    }

    destroy_index_table(itable);
    return 0;
}

/**
 * Fold the result of DFT.  Uses the input array in a corruptive way.
 */
static inline void
solve(double complex *input, size_t num_stages)
{
    for (size_t s = 1; s <= num_stages; s++) {
        unsigned int N = 1 << s;        // Unit of batterfly
        double a = 2.0 * M_PI / (double)N;
        for (int i = 0; i < (1 << num_stages) / N; i++) {
            for (int j = 0; j < N / 2; j++) {
                double real = cos(a * j);
                double imag = sin(a * j);
                unsigned int m = i * N + j;
                unsigned int n = m + N / 2;
                double complex delta = CMPLX(real, imag) * input[n];
                input[n] = input[m] - delta;
                input[m] = input[m] + delta;
            }
        }
    }
}

/**
 * Fast Fourier Transform.
 *
 * @param sample    the signal samples.
 * @param count     the number of samples.
 * @param result    the result of transform.
 */
int
fft(double *samples, size_t count, double complex *result)
{
    /*
     * Expand and align the buffer size to power of 2, which is the
     * assumption of FFT.
     */
    size_t exp = to_exp(count - 1);
    double complex *buf = calloc(1 << exp, sizeof(double complex));
    if (buf == NULL) {
        return -1;
    }

    if (copy_and_sort(buf, exp, samples, count) < 0) {
        return -1;
    }

    solve(buf, exp);

    for (int i = 0; i < count; i++) {
        result[i] = buf[i];
    }
    free(buf);

    return 0;
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

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

/**
 * Copies the given samples so that they are aligned for the butterfly
 * calculation.
 *
 * @param dest  the destination buffer.
 * @param exp   the size of the destination in the power of two.
 * @param src   the original samples.
 * @param count the count of samples.
 */
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

    /* Sort the samples for the butterfly calculation. */
    for (int i = 0; i < count; i++) {
        dest[itable[i]] = src[i];
    }

    destroy_index_table(itable);
    return 0;
}

/**
 * Fast Fourier Transform.
 *
 * @param input         the signal samples aligned in power of 2.
 * @param num_stages    the number of stages.
 */
static inline void
fft(double complex *input, size_t num_stages)
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

static int
do_fft(double *samples, size_t count)
{
    /*
     * Expand and align the buffer size to power of 2, which is the
     * precondition of FFT.
     */
    size_t exp = to_exp(count - 1);
    size_t exp_len = 1 << exp;

    double complex *buf = calloc(exp_len, sizeof(double complex));
    if (buf == NULL) {
        return -1;
    }

    if (copy_and_sort(buf, exp, samples, count) < 0) {
        free(buf);
        return -1;
    }

    fft(buf, exp);

    double res = (double)count / (double)exp_len;
    /* Output only the left channel */
    for (int i = 0; i < count / 2; i++) {
        printf("%f %f %f 0 0\n", res * i, cabs(buf[i]), carg(buf[i]));
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
    double *tmp = calloc(len, sizeof(double));
    printf("# %zu samples to be processed.\n", len);

    wave_single_channel(handle, rbuf, tmp, len, 0);

    do_fft(tmp, len);

    free(tmp);
    wave_free_read_buffer(rbuf);
    wave_close(handle);

    return EXIT_SUCCESS;
}


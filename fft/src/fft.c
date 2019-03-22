#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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
fold(size_t exp, double complex *input, double complex *output, size_t length)
{
    for (int i = 0; i < length / N; i++) {
        for (j = 0; j < N / 2; j++) {
            int m = i * N + j;
            int n = m + N / 2;
            double complex delta = omegaN[m] * input[n];
            output[m] = input[m] + delta;
            output[n] = input[m] - delta;
        }
    }
}

static void
solve(double complex *input, double complex *output, size_t length)
{
    // TODO: Buffer management
    double complex *tmp = malloc();

    for (size_t exp = 2; exp < SIZE_OF_PROB; exp++) {
        fold(exp, tmp, output, length);
    }
}

#define CH2RAW(i, ch, offset)   ((i) * (ch) + (offset))

static void
copy_and_sort(size_t ch, double *dest, size_t exp, double *src, size_t len)
{
    size_t *itable = create_index_table(exp);
    if (itable == NULL) {
        //TODO: Error report
        printf("ERROR\n");
        return;
    }

    for (int i = 0; i < len; i++) {
        for (int j = 0; j < ch; j++) {
            dest[CH2RAW(itable[i], ch, j)] = src[CH2RAW(i, ch, j)];
        }
    }

    destroy_index_table(itable);
}

void
fft(wave_handle_t *handle, wave_buffer_t *orig, double complex *result)
{
    size_t ch = handle->num_channels;
    size_t len = handle->sample_rate;
    double *ptr = orig->buf;
    size_t exp;

    /*
     * Expand and align the buffer size to power of 2, which is the
     * assumption of FFT.
     */
    for (exp = 0; len > 0; len >>= 1, exp++) ;

    size_t extended = 1 << exp;
    double buf = calloc(extended, sizeof(double));
    if (buf == NULL) {
        //TODO: Error report
        printf("ERROR\n");
        return;
    }

    copy_and_sort(ch, buf, exp, ptr, len);

    /* DFT (N = 2) */
    const int N = 2;
    double complex *tmp = malloc(sizeof(double complex) * extended);
    if (tmp == NULL) {
        //TODO: Error report
        printf("ERROR\n");
        return;
    }

    for (int i = 0; i < extended; i += N) {
        dft(ch, &buf[i], N, &tmp[i]);
    }

    /* Fold */
    solve(tmp, result, exp);
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

    double complex *result = malloc(sizeof(double complex) * length);
    fft(handle, buffer, length, result);

    /* Output only the left channel */
    for (int i = 0; i < length / 4; i++) {
        printf("%d %f %f %f %f\n", i,
                cabs(result[i * 2]), carg(result[i * 2]),
                cabs(result[i * 2 + 1]), carg(result[i * 2 + 1]));
    }

    free(result);
    wave_free_buffer(buffer);
    wave_close(handle);

    return EXIT_SUCCESS;
}

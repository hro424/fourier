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
    int ch = handle->num_channels;
    int sample_rate = handle->sample_rate;

    for (int k = 0; k < sample_rate; k++) {
        double a = 2.0 * M_PI * k / sample_rate;

        for (int c = 0; c < ch; c++) {
            int index = k * ch + c;
            if (result_real != NULL) {
                result_real[index] = 0.0;
            }

            if (result_imag != NULL) {
                result_imag[index] = 0.0;
            }

            /* Process only the left or right side channel at once */
            for (int n = 0; n < sample_rate; n++) {
                int nn = n * ch + c;
                if (result_real != NULL) {
                    double real = cos(a * n);
                    result_real[index] += real * buf->buffer[nn];
                }

                if (result_imag != NULL) {
                    double imag = sin(a * n);
                    result_imag[index] += imag * buf->buffer[nn];
                }
            }
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
    ssize_t length = wave_read(handle, buffer);
    printf("# %zd samples read.\n", length);

    double *real = malloc(sizeof(double) * length);
    double *imag = malloc(sizeof(double) * length);
    dft(handle, buffer, length, real, imag);

    /* Output only the left channel */
    for (int i = 0; i < length / 4; i++) {
        int ii = i * 2;
        double amp = hypot(real[ii], imag[ii]);
        printf("%d %f\n", i, amp);
    }

    free(imag);
    free(real);
    wave_free_buffer(buffer);
    wave_close(handle);

    return EXIT_SUCCESS;
}

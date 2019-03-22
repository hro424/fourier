#ifndef FOURIER_WAVE_H
#define FOURIER_WAVE_H

#include <stdlib.h>
#include <stdint.h>

#define BITS_PER_SAMPLE_8   8
#define BITS_PER_SAMPLE_16  16

typedef struct wave_handle
{
    /**
     * The file descriptor of a wave file
     */
    int fd;

    /**
     * The size of data in bytes.
     */
    uint32_t length;

    /**
     * The number of channels
     */
    uint16_t num_channels;

    /**
     * Sampling rate
     */
    uint32_t sample_rate;

    /**
     * Size of wave data in 1 second in bytes.
     * (sample_rate * num_channels * bits_per_sample / 8)
     */
    uint32_t byte_rate;

    /**
     * Size of a block of wave data in bytes.
     * (num_channels * bits_per_sample / 8)
     */
    uint16_t block_size;

    /**
     * Bits per sample
     */
    uint16_t bits_per_sample;
} wave_handle_t;

typedef struct wave_buffer
{
    /* Number of samples */
    size_t length;
    /* Array of samples */
    double *buffer;
} wave_buffer_t;

/**
 * Opens the wave file and creates a handle to it.
 */
wave_handle_t *wave_open(const char *path, int mode);

/**
 * Closes the given handle.
 */
void wave_close(wave_handle_t *handle);

/**
 * Allocates a buffer and assigns to the handle.
 *
 * @param handle    the handle.
 * @param sec       the duration.
 */
wave_buffer_t *wave_alloc_buffer(wave_handle_t *handle, int sec);

/**
 * Releases the buffer.
 */
void wave_free_buffer(wave_buffer_t *buf);

/**
 * Reads data in the wave file and fills the buffer with it.
 */
ssize_t wave_read(wave_handle_t *handle, wave_buffer_t *buf);

/**
 * Writes the data in the buffer to the wave file.
 */
ssize_t wave_write(wave_handle_t *handle, const wave_buffer_t *buf);

#endif /* FOURIER_WAVE_H */

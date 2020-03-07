#ifndef FOURIER_WAVE_H
#define FOURIER_WAVE_H

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

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

static inline size_t
wave_ch(wave_handle_t *h)
{
    return h->num_channels;
}

static inline uint32_t
wave_sr(wave_handle_t *h)
{
    return h->sample_rate;
}

static inline uint32_t
wave_br(wave_handle_t *h)
{
    return h->byte_rate;
}

static inline size_t
wave_bsize(wave_handle_t *h)
{
    return h->block_size;
}

typedef struct wave_buffer
{
    size_t length;
    double *buffer;
} wave_buffer_t;

typedef struct wave_read_buffer
{
    size_t length;
    uint8_t *body;
} wave_read_buffer_t;

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
 * Allocates a buffer for wave data.
 *
 * @param handle    the handle of the wave file.
 * @param sec       the size of the buffer in seconds.
 */
wave_read_buffer_t *wave_alloc_read_buffer(wave_handle_t *handle, unsigned int sec);

/**
 * Release the buffer.
 */
void wave_free_read_buffer(wave_read_buffer_t *buf);

ssize_t wave_rawread(wave_handle_t *handle, wave_read_buffer_t *buf);

ssize_t wave_single_channel(wave_handle_t *h, wave_read_buffer_t *buf,
                            double *dest, size_t len, unsigned int ch);

/**
 * Writes the data in the buffer to the wave file.
 */
ssize_t wave_write(wave_handle_t *handle, const wave_buffer_t *buf);

#endif /* FOURIER_WAVE_H */

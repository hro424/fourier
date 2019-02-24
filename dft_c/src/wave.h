#ifndef FOURIER_WAVE_H
#define FOURIER_WAVE_H

#include <stdlib.h>
#include <stdint.h>

typedef struct wave_handle
{
    int fd;
    uint32_t length;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_size;
    uint16_t bits_per_sample;
} wave_handle_t;

typedef struct wave_buffer
{
    size_t length;
    uint8_t *buffer;
} wave_buffer_t;

wave_handle_t *wave_open(const char *path, int mode);
void wave_close(wave_handle_t *handle);
wave_buffer_t *wave_alloc_buffer(wave_handle_t *handle, int sec);
void wave_free_buffer(wave_buffer_t *buf);
ssize_t wave_read(wave_handle_t *handle, wave_buffer_t *buf);
ssize_t wave_write(wave_handle_t *handle, const wave_buffer_t *buf);

#endif /* FOURIER_WAVE_H */

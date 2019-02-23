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


wave_handle_t *wave_open(const char *path, int mode);
void wave_close(wave_handle_t *handle);
ssize_t wave_read(wave_handle_t *handle, void *buf, size_t len);
ssize_t wave_write(wave_handle_t *handle, const void *buf, size_t len);

#endif /* FOURIER_WAVE_H */

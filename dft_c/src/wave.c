/**
 * Wave file reader/writer
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "wave.h"

/*
 * Wave file format
 *
 * | 4B | 'RIFF' |
 * | 4B | File size in bytes |
 * | 4B | 'WAVE' |
 * | 4B | Tag 1 |
 * | 4B | Data length 1 |
 * | n | Data 1 |
 *
 *
 * fmt chunk
 * | 4B | 'fmt ' |
 * | 4B | Length |
 * | 2B | Format ID |
 * | 2B | Number of channels |
 * | 4B | Sampling rate (Hz) |
 * | 4B | Speed (B/sec) |
 * | 2B | Block size (B/sample*ch) |
 * | 2B | Bits per sample (bit/sample) |
 * | 2B | Extension length |
 * | n byte | Extension |
 *
 * data chunk
 * | 4B | 'data' |
 * | 4B | Length |
 * | Length | Wave data |
 */

typedef struct riff_chunk
{
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
} riff_chunk_t;

typedef struct fmt_chunk_header
{
    char chunk_id[4];
    uint32_t chunk_size;
} fmt_chunk_header_t;

typedef struct fmt_chunk_body
{
    uint16_t format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_size;
    uint16_t bits_per_sample;
} fmt_chunk_body_t;

typedef struct data_chunk_header
{
    char chunk_id[4];
    uint32_t chunk_size;
} data_chunk_header_t;

wave_handle_t *
wave_open(const char *path, int mode)
{
    ssize_t sz;
    riff_chunk_t riff_chunk;
    fmt_chunk_header_t fmt_chunk_header;
    data_chunk_header_t data_chunk_header;

    int fd = open(path, mode);

    if (fd < 0) {
        goto error;
    }

    sz = read(fd, &riff_chunk, sizeof(riff_chunk_t));
    if (sz < sizeof(riff_chunk_t)) {
        close(fd);
        goto error;
    }

    if (strncmp(riff_chunk.chunk_id, "RIFF", 4) != 0 ||
        strncmp(riff_chunk.format, "WAVE", 4) != 0) {
        close(fd);
        goto error;
    }

    sz = read(fd, &fmt_chunk_header, sizeof(fmt_chunk_header_t));
    if (sz < sizeof(fmt_chunk_header_t)) {
        close(fd);
        goto error;
    }

    if (strncmp(fmt_chunk_header.chunk_id, "fmt ", 4) != 0) {
        close(fd);
        goto error;
    }

    uint8_t buf[BUFSIZ];
    sz = read(fd, buf, fmt_chunk_header.chunk_size);
    if (sz < fmt_chunk_header.chunk_size) {
        close(fd);
        goto error;
    }

    fmt_chunk_body_t *ptr = (fmt_chunk_body_t *)buf;

    sz = read(fd, &data_chunk_header, sizeof(data_chunk_header_t));
    if (sz < sizeof(data_chunk_header_t)) {
        close(fd);
        goto error;
    }

    if (strncmp(data_chunk_header.chunk_id, "data", 4) != 0) {
        close(fd);
        goto error;
    }

    wave_handle_t *handle = malloc(sizeof(wave_handle_t));
    if (handle == NULL) {
        close(fd);
        goto error;
    }

    handle->fd = fd;
    handle->length = data_chunk_header.chunk_size;
    handle->num_channels = ptr->num_channels;
    handle->sample_rate = ptr->sample_rate;
    handle->byte_rate = ptr->byte_rate;
    handle->block_size = ptr->block_size;
    handle->bits_per_sample = ptr->bits_per_sample;

    return handle;

error:
    return NULL;
}

void
wave_close(wave_handle_t *handle)
{
    int fd = handle->fd;
    free(handle);
    if (fd > 0) {
        close(fd);
    }
}

wave_buffer_t *
wave_alloc_buffer(wave_handle_t *handle, int sec)
{
    wave_buffer_t *buf = NULL;
    void *ptr = malloc(handle->sample_rate * sec);
    if (ptr != NULL) {
        buf = malloc(sizeof(wave_buffer_t));
        if (buf == NULL) {
            free(ptr);
        }
        else {
            buf->length = handle->sample_rate * sec;
            buf->buffer = ptr;
        }
    }

    return buf;
}

void
wave_free_buffer(wave_buffer_t *buf)
{
    free(buf->buffer);
    free(buf);
}

ssize_t
wave_read(wave_handle_t *handle, wave_buffer_t *buf)
{
    return read(handle->fd, buf->buffer, buf->length);
}

ssize_t
wave_write(wave_handle_t *handle, const wave_buffer_t *buf)
{
    return write(handle->fd, buf->buffer, buf->length);
}

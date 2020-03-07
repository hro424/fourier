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

#define CHUNK_ID_SIZE       4
#define CHUNK_ID_RIFF       "RIFF"
#define CHUNK_ID_FMT        "fmt "
#define CHUNK_ID_DATA       "data"
#define FORMAT_TYPE_SIZE    4
#define FORMAT_TYPE_WAVE    "WAVE"

typedef struct riff_chunk
{
    char chunk_id[CHUNK_ID_SIZE];
    uint32_t chunk_size;
    char format[FORMAT_TYPE_SIZE];
} riff_chunk_t;

typedef struct fmt_chunk_header
{
    char chunk_id[CHUNK_ID_SIZE];
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
    char chunk_id[CHUNK_ID_SIZE];
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

    if (strncmp(riff_chunk.chunk_id, CHUNK_ID_RIFF, CHUNK_ID_SIZE) != 0 ||
        strncmp(riff_chunk.format, FORMAT_TYPE_WAVE, FORMAT_TYPE_SIZE) != 0) {
        close(fd);
        goto error;
    }

    sz = read(fd, &fmt_chunk_header, sizeof(fmt_chunk_header_t));
    if (sz < sizeof(fmt_chunk_header_t)) {
        close(fd);
        goto error;
    }

    if (strncmp(fmt_chunk_header.chunk_id, CHUNK_ID_FMT, CHUNK_ID_SIZE) != 0) {
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

    if (strncmp(data_chunk_header.chunk_id, CHUNK_ID_DATA, CHUNK_ID_SIZE) != 0) {
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
    // Total number of samples in the given duration
    size_t length = handle->num_channels * handle->sample_rate * sec;
    wave_buffer_t *buf = NULL;
    double *ptr = malloc(length * sizeof(double));
    if (ptr != NULL) {
        buf = malloc(sizeof(wave_buffer_t));
        if (buf == NULL) {
            free(ptr);
        }
        else {
            buf->length = length;
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

#define BITS_PER_BYTE    8

ssize_t
wave_read(wave_handle_t *handle, wave_buffer_t *buf)
{
    ssize_t sz = 0;

    if (handle == NULL || buf == NULL) {
        goto exit;
    }

    if (handle->bits_per_sample == BITS_PER_SAMPLE_8) {
        size_t bufsz = buf->length * handle->bits_per_sample / BITS_PER_BYTE;
        uint8_t *tmp = malloc(bufsz);
        if (tmp == NULL) {
            goto exit;
        }

        sz = read(handle->fd, tmp, bufsz);
        for (ssize_t i = 0; i < sz; i++) {
            buf->buffer[i] = (double)tmp[i] / (double)UINT8_MAX;
        }
        free(tmp);
    }
    else if (handle->bits_per_sample == BITS_PER_SAMPLE_16) {
        // wave_buffer_t already counts the number of channels.
        // So, the size of sample in bytes is taken into account here.
        size_t bufsz = buf->length * handle->bits_per_sample / BITS_PER_BYTE;
        int16_t *tmp = malloc(bufsz);
        if (tmp == NULL) {
            goto exit;
        }

        sz = read(handle->fd, tmp, bufsz);
        // Adjust the count to the int16_t array.
        sz /= sizeof(int16_t);
        for (ssize_t i = 0; i < sz; i++) {
            buf->buffer[i] = (double)tmp[i] / ((double)INT16_MAX + 1.0);
        }
        free(tmp);
    }
exit:
    return sz;
}

wave_read_buffer_t *
wave_alloc_read_buffer(wave_handle_t *h, unsigned int sec)
{
    wave_read_buffer_t *buf = NULL;
    size_t length = h->byte_rate * sec;

    uint8_t *ptr = malloc(length);
    if (ptr != NULL) {
        buf = malloc(sizeof(wave_read_buffer_t));
        if (buf == NULL) {
            free(ptr);
        }
        else {
            buf->length = length;
            buf->body = ptr;
        }
    }

    return buf;
}

void
wave_free_read_buffer(wave_read_buffer_t *buf)
{
    free(buf->body);
    free(buf);
}

ssize_t
wave_rawread(wave_handle_t *h, wave_read_buffer_t *buf)
{
    return read(h->fd, buf->body, buf->length);
}

ssize_t
wave_single_channel(wave_handle_t *h, wave_read_buffer_t *buf,
                    double *dest, size_t len, unsigned int ch)
{
    ssize_t l = -1;

    if (h == NULL || dest == NULL || buf == NULL) {
        goto exit;
    }

    size_t nch = h->num_channels;
    size_t block_size = h->block_size;

    if (!(ch < nch)) {
        goto exit;
    }

    if (buf->length / block_size < len) {
        l = buf->length / block_size;
    }
    else {
        l = len;
    }

    if (h->bits_per_sample == BITS_PER_SAMPLE_8) {
        for (size_t i = 0; i < (size_t)len; i++) {
            dest[i] = (double)buf->body[i * nch + ch] / (double)UINT8_MAX;
        }
    }
    else if (h->bits_per_sample == BITS_PER_SAMPLE_16) {
        int16_t *ptr = (int16_t *)buf->body;
        for (size_t i = 0; i < (size_t)len; i++) {
            dest[i] = (double)ptr[i * nch + ch] / ((double)INT16_MAX + 1.0);
        }
    }

exit:
    return l;
}

ssize_t
wave_write(wave_handle_t *handle, const wave_buffer_t *buf)
{
    //return write(handle->fd, buf->buffer, buf->length);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "wave.h"

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    wave_handle_t *handle;

    printf("open %s\n", argv[1]);
    handle = wave_open(argv[1], O_RDONLY);
    if (handle == NULL) {
        return EXIT_FAILURE;
    }

    printf("length %u\n", handle->length);
    printf("num_channels %u\n", handle->num_channels);
    printf("sample_rate %u\n", handle->sample_rate);
    printf("byte_rate %u\n", handle->byte_rate);
    printf("block_size %u\n", handle->block_size);
    printf("bits_per_sample %u\n", handle->bits_per_sample);

    wave_close(handle);

    return EXIT_SUCCESS;
}

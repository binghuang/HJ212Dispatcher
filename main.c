#include <stdio.h>
#include <stdlib.h>

#include "main.h"

int main(int argc, char **argv)
{
    int fd = -1;

    fd = init_gather("lo", 9090);
    if (fd < 0) {
        printf("Init gather failed\n");
        exit(EXIT_FAILURE);
    }

    void *handle = NULL;
    handle = start_gather(fd);
    if (!handle) {
        printf("Start gather failed\n");
        deinit_gather(handle, fd);
    }

    deinit_gather(handle, fd);

    return 0;
}
#include <string.h>
#include <pthread.h>

#include "hj212.h"

#define MAP_SIZE 2048

static struct {
    char mn[MN_SIZE];
    int fd;
} mn_fd[MAP_SIZE];

static pthread_mutex_t map_mutex;
static int map_index;

void add_mn_fd(char * mn, int fd)
{
    int i;

    if (mn == NULL || fd < 0)
        return;

    pthread_mutex_lock(&map_mutex);

    for (i = 0; i < MAP_SIZE; i++) {
        if (strlen(mn_fd[i].mn) == 0)
            continue;
        if (strlen(mn_fd[i].mn) == strlen(mn) &&
            strncmp(mn_fd[i].mn, mn, strlen(mn)) == 0 &&
            mn_fd[i].fd == fd)
            return;
    }

    for (i = 0; i < MAP_SIZE; i++)
        if (strlen(mn_fd[i].mn) == 0)
            break;

    memcpy(mn_fd[i++].mn, mn, strlen(mn));
    mn_fd[i++].fd = fd;

    pthread_mutex_unlock(&map_mutex);
}

void del_mn_fd(int fd)
{
    int i;

    if (fd < 0)
        return;

    pthread_mutex_lock(&map_mutex);

    for (i = 0; i < MAP_SIZE; i++) {
        if (strlen(mn_fd[i].fd) == fd) {
            memcpy(mn_fd[i].mn, 0, strlen(MN_SIZE));
            mn_fd[i].fd = 0;
        }
    }

    pthread_mutex_unlock(&map_mutex);
}

int find_fd(char * mn)
{
    int i;

    if (mn == NULL)
        return;

    pthread_mutex_lock(&map_mutex);

    for (i = 0; i < MAP_SIZE; i++) {
        if (strlen(mn_fd[i].mn) == 0)
            continue;
        if (strlen(mn_fd[i].mn) == strlen(mn) &&
            strncmp(mn_fd[i].mn, mn, strlen(mn)) == 0)
            return mn_fd[i].fd;
    }

    pthread_mutex_unlock(&map_mutex);

    return -1;
}
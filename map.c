#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "hj212.h"

#define MAP_SIZE 2048

static struct {
    char mn[MN_SIZE];
    int fd;
} mn_fd[MAP_SIZE];

static pthread_mutex_t map_mutex;

int init_mn_fd(void)
{
    int i;

    for (i = 0; i < MAP_SIZE; i++) {
        mn_fd[i].fd = -1;
        memset(mn_fd[i].mn, 0, MN_SIZE);
    }

    if (pthread_mutex_init(&map_mutex, NULL) < 0) {
        perror("Failed to init mutex");
        return -1;
    }

    return 0;
}

void show_mn_fd(void)
{
    int i;

    for (i = 0; i < MAP_SIZE; i++)
        if (strlen(mn_fd[i].mn) != 0)
            printf("%d: %s@%d\n", i, mn_fd[i].mn, mn_fd[i].fd);
}

void add_mn_fd(char * mn, int fd)
{
    int i;

    if (mn == NULL || fd < 0)
        return;

    printf("%s, %d\n", __FUNCTION__, __LINE__);
    pthread_mutex_lock(&map_mutex);

    printf("%s, %d\n", __FUNCTION__, __LINE__);
    for (i = 0; i < MAP_SIZE; i++) {
        if (strlen(mn_fd[i].mn) == 0)
            continue;
        if (strlen(mn_fd[i].mn) == strlen(mn) &&
            strncmp(mn_fd[i].mn, mn, strlen(mn)) == 0 &&
            mn_fd[i].fd == fd) {
                pthread_mutex_unlock(&map_mutex);
                return;
            }
    }

    printf("%s, %d\n", __FUNCTION__, __LINE__);
    for (i = 0; i < MAP_SIZE; i++)
        if (strlen(mn_fd[i].mn) == 0)
            break;

    printf("%s, %d\n", __FUNCTION__, __LINE__);
    memcpy(mn_fd[i].mn, mn, strlen(mn));
    mn_fd[i].fd = fd;

    pthread_mutex_unlock(&map_mutex);
}

void del_mn_fd(int fd)
{
    int i;

    if (fd < 0)
        return;

    pthread_mutex_lock(&map_mutex);

    for (i = 0; i < MAP_SIZE; i++) {
        if (mn_fd[i].fd == fd) {
            memset(mn_fd[i].mn, 0, MN_SIZE);
            mn_fd[i].fd = 0;
        }
    }

    pthread_mutex_unlock(&map_mutex);
}

int find_fd(char * mn)
{
    int i;

    if (mn == NULL)
        return -1;

    pthread_mutex_lock(&map_mutex);

    for (i = 0; i < MAP_SIZE; i++) {
        if (strlen(mn_fd[i].mn) == 0)
            continue;
        if (strlen(mn_fd[i].mn) == strlen(mn) &&
            strncmp(mn_fd[i].mn, mn, strlen(mn)) == 0) {
                pthread_mutex_unlock(&map_mutex);
                return mn_fd[i].fd;
            }
    }

    pthread_mutex_unlock(&map_mutex);

    return -1;
}
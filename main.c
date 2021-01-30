#include <stdio.h>
#include <pthread.h>

extern void * start_server(void *arg);
extern void * start_client(void *arg);

int main(int argc, char **argv)
{
    int ret = -1;
    pthread_t gather, older, newer;

    ret = pthread_create(&gather, NULL, start_server, NULL);
    if (ret < 0) {
        perror("cannot create gather:");
        return -1;
    }

    ret = pthread_create(&older, NULL, start_client, NULL);
    if (ret < 0) {
        perror("cannot create older:");
        return -1;
    }

    ret = pthread_create(&newer, NULL, start_client, NULL);
    if (ret < 0) {
        perror("cannot create newer:");
        return -1;
    }

    pthread_join(gather, NULL);
    pthread_join(older, NULL);
    pthread_join(newer, NULL);

    return 0;
}
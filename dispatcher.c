#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>

#include "map.h"
#include "hj212.h"
#include "main.h"

#define DISPATCHER_NAME_LEN     8

typedef struct
{
    char name[DISPATCHER_NAME_LEN];
    char serverAddr[INET_ADDRSTRLEN];
    short port;
    int fd;
} dispatcher;

static int init_dispatcher_socket(char *servaddr, short port);
static void * dispatcher_thread(void *arg)
{
    dispatcher *d = (dispatcher *)arg;
    int fd = -1;
    char buf[1200];
    char mn[MN_SIZE];
    int len, device_fd;
    int ret = -1;

    fd = init_dispatcher_socket(d->serverAddr, d->port);
    if (fd < 0)
        return (void *)-1;
    d->fd = fd;

    while (1) {
        memset(buf, 0, 1200);
        if ((len = read(fd, buf, 1200)) <= 0) {
            close(fd);
            d->fd = -1;
            return (void *)-1;
        }

        memset(mn, 0, MN_SIZE);
        ret = hj212_valid(buf, mn);
        if (ret != 0)
            continue;
        
		// show_mn_fd();
        device_fd = find_fd(mn);
        gather_send(device_fd, buf, len);
    }

    close(fd);
    d->fd = -1;
    return (void *)0;
}

static void * dispatcher_thread_create(void *arg)
{
    pthread_t tid;

    while (1) {
        if (pthread_create(&tid, NULL, dispatcher_thread, arg)  < 0) {
            perror("Failed to create dispatcher");
            return (void *)-1;
        }

        pthread_join(tid, NULL);
        sleep(2);
    }

    return (void *)0;
}

static dispatcher photon = { .fd = -1, };
static dispatcher other = { .fd = -1 };

int init_dispatcher(const char *name, char *servaddr, short port)
{
    pthread_t tid;
    pthread_attr_t attr;

    if (pthread_attr_init(&attr) < 0) {
        perror("Failed to init thread attr");
        return -1;
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) < 0) {
        perror("Failed to set deatach state");
        return -1;
    }
    if (name == NULL || name[0] == 0 || strlen(name) == 0)
        return -1;

    if (servaddr == NULL || servaddr[0] == 0 || strlen(servaddr) == 0)
        return -1;

    if (strlen(name) == strlen(DISPATCHER_NAME_PHOTON) && strcmp(name, DISPATCHER_NAME_PHOTON) == 0) {
        photon.port = port;
        memset(photon.serverAddr, 0, INET_ADDRSTRLEN);
        memcpy(photon.serverAddr, servaddr, strlen(servaddr));
        memset(photon.name, 0, DISPATCHER_NAME_LEN);
        memcpy(photon.name, name, DISPATCHER_NAME_LEN);
        if (pthread_create(&tid, NULL, dispatcher_thread_create, (void *)&photon)  < 0) {
            perror("Failed to create dispatcher");
            return -1;
        }
    } else {
        other.port = port;
        memset(other.serverAddr, 0, INET_ADDRSTRLEN);
        memcpy(other.serverAddr, servaddr, strlen(servaddr));
        memset(other.name, 0, DISPATCHER_NAME_LEN);
        memcpy(other.name, name, strlen(name));
#if 0
        if (pthread_create(&tid, NULL, dispatcher_thread_create, (void *)&other)  < 0) {
            perror("Failed to create dispatcher");
            return -1;
        }
#endif
    }
    pthread_attr_destroy(&attr);

    return 0;
}

void dispatcher_send_photon(const char *buf, int len)
{
    int i;
    int ret;

    if (photon.fd >= 0) {
        ret = write(photon.fd, buf, len);
        if (ret < 0)
            perror("Failed to dispatch");
    }
}

void dispatcher_send_other(const char *buf, int len)
{
    int fd = -1;
    int ret = -1;

    fd = init_dispatcher_socket(other.serverAddr, other.port);
    if (fd < 0)
        return

    ret = write(fd, buf, len);
    if (ret < 0)
        perror("Failed to dispatch");

    close(fd);
}

static int init_dispatcher_socket(char *ipaddr, short port)
{
    int fd = -1;
    char error_info[64];
    struct sockaddr_in addr;
    int ret = -1;

    if (NULL == ipaddr || 0 == ipaddr[0] || strlen(ipaddr) > IF_NAMESIZE) {
        fprintf(stderr, "Ip addr is null\n");
        goto error_exit;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Failed to create dispatcher socket");
        goto close_fd;
    }

    memset((void *)&addr, 0, sizeof(struct sockaddr_in)); 
    addr.sin_family = AF_INET;
    if (inet_aton(ipaddr, &addr.sin_addr) < 0) {
        memset(error_info, 0, 64);
        snprintf(error_info, 64, "Ip[%s] fault", ipaddr);
        perror(error_info);
        goto close_fd;
    }
    addr.sin_port = htons(port);

    ret = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (ret < 0) {
        memset(error_info, 0, 64);
        snprintf(error_info, 64, "Failed to connect to %s:%d", ipaddr, port);
        perror(error_info);
        goto close_fd;
    }

    return fd;

close_fd:
    close(fd);
error_exit:
    return ret;
}

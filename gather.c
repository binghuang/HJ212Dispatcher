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

void gather_send(int device_fd, const char *buf, int len)
{
    int ret;

    ret = write(device_fd, buf, len);
    if (ret < 0)
        perror("Failed to send");
}

static void *machine_thread(void *arg)
{
    int fd = (int)arg;
    char buf[1200];
    char mn[MN_SIZE];
    int ret = -1;
    int len = 0;

    while (1) {
        memset(buf, 0, 1200);
        if ((len = read(fd, buf, 1200)) <= 0) {
            del_mn_fd(fd);
            close(fd);
            return (void *)-1;
        }

        memset(mn, 0, MN_SIZE);
        ret = hj212_valid(buf, mn); 
        if (ret != 0)
            continue;

        add_mn_fd(mn, fd);

        dispatcher_send(buf, len);
    }
    return (void *)0;
}

static void * gather_thread(void *arg)
{
    int fd = (int)arg;
    int client_fd = -1;
    struct sockaddr_in client_addr;
    socklen_t addr_len; 
    char info[64];
    pthread_t tid;
    pthread_attr_t attr;
    int ret = -1;
    
    while (1) {
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        client_fd = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Failed to accept connection");
            continue;
        }

        memset(info, 0, 64);

        if (pthread_attr_init(&attr) < 0) {
            perror("Failed to init thread attr");
            continue;
        }
        if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) < 0) {
            perror("Failed to set deatach state");
            continue;
        }
        ret = pthread_create(&tid, &attr, machine_thread, (void *)client_fd); 
        if (ret < 0) {
            perror("Failed to create machine thread");
            if (pthread_attr_destroy(&attr) < 0)
                perror("Failed to destory thread attr");
            continue;
        }
        pthread_attr_destroy(&attr);
    }

    return (void *)0;
}

void * start_gather(int fd)
{
    pthread_t tid;

    if (pthread_create(&tid, NULL, gather_thread, (void *)fd)  < 0) {
        perror("Failed to create gather");
        return (void *)0;
    }

    return (void *)tid;
}

int init_gather(char *ifname, short port)
{
    int fd = -1;
    char error_info[64];
    struct sockaddr_in addr;
    struct ifreq ifreq;
    int ret = -1;

    if (NULL == ifname || 0 == ifname[0] || strlen(ifname) > IF_NAMESIZE) {
        fprintf(stderr, "ifname is null\n");
        goto error_exit;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Failed to create gather socket");
        goto close_fd;
    }

    memset((void *)&ifreq, 0, sizeof(struct ifreq));
    memcpy((void *)ifreq.ifr_name, ifname, strlen(ifname));
    ret = ioctl(fd, SIOCGIFADDR, &ifreq);
    if (ret < 0) {
        snprintf(error_info, 64, "Failed to request %s address", ifname);
        perror(error_info);
        goto close_fd;
    }

    memset((void *)&addr, 0, sizeof(struct sockaddr_in)); 
    memcpy(&addr, &ifreq.ifr_addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    ret = bind(fd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (ret < 0) {
        snprintf(error_info, 64, "Failed to bind %s:%d", inet_ntoa(addr.sin_addr), port);
        perror(error_info);
        goto close_fd;
    }

    ret = listen(fd, 1024);
    if (ret < 0) {
        perror("Faild to listen");
        goto close_fd;
    }

    return 0;

close_fd:
    close(fd);
error_exit:
    return ret;
}

void deinit_gather(void *handle, int fd)
{
    pthread_t tid;

    if (handle) {
        tid = (pthread_t)handle; 
        pthread_join(tid, NULL);
    }

    if (fd >= 0)
        close(fd);
}
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
#include <poll.h>

#include "map.h"
#include "hj212.h"
#include "main.h"

void gather_send(int device_fd, const char *buf, int len)
{
    int ret;

    if (device_fd >= 0) {
        ret = write(device_fd, buf, len);
        if (ret < 0)
            perror("Failed to send");
    }
}

static void dispatch(int fd)
{
    char buf[1200];
    char mn[MN_SIZE];
    int ret = -1;
    int len = 0;

    memset(buf, 0, 1200);
    if ((len = read(fd, buf, 1200)) <= 0) {
        del_mn_fd(fd);
        close(fd);
        return;
    }

    dispatcher_send_other(buf, len);
    dispatcher_send_photon(buf, len);

    memset(mn, 0, MN_SIZE);
    ret = hj212_valid(buf, mn); 
    if (ret == 0)
        add_mn_fd(mn, fd);
}

struct hj212_polls {
    struct  pollfd *fds;
    int nums;
};

static void * gather_thread(void *arg)
{
    int fd = (int)arg;
    int client_fd = -1;
    struct sockaddr_in client_addr;
    socklen_t addr_len; 
    int ret = -1;
    struct hj212_polls polls;
    struct pollfd *fds;
    int fd_ind = -1;

    polls.nums = 1024;
    polls.fds = malloc(sizeof(struct hj212_polls) * polls.nums);
    if (!polls.fds) {
        printf("memory not sufficent!!\n");
        return (void *)-1;
    }

    memset(polls.fds, 0, polls.nums * sizeof(struct  pollfd));
    for (fd_ind = 0; fd_ind < polls.nums; fd_ind++)
        polls.fds[fd_ind].fd = -1;

    polls.fds[0].fd = fd;
    polls.fds[0].events = POLLIN;

    while (1) {
        ret = poll(polls.fds, polls.nums, 1000);
        if (polls.fds[0].revents & POLLIN) {
            client_fd = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
            if (client_fd >= 0) {
                for (fd_ind = 0; fd_ind < polls.nums; fd_ind++)
                    if (polls.fds[fd_ind].fd == -1)
                        break;
                if (fd_ind == polls.nums) {
                    polls.nums += 1024;
                    fds = malloc(sizeof(struct hj212_polls) * polls.nums);
                    if (fds) {
                        memset(fds, 0, polls.nums * sizeof(struct  pollfd));
                        for (fd_ind = 0; fd_ind < polls.nums; fd_ind++)
                            polls.fds[fd_ind].fd = -1;
                        memcpy(fds, polls.fds, fd_ind);
                    } else {
                        polls.nums -= 1024;
                        printf("Cannot increase poll space!!\n"); 
                    }

                }
                if (fd_ind != polls.nums) {
                    polls.fds[fd_ind].fd = client_fd;
                    polls.fds[fd_ind].events = POLLIN;
                }
            } else
                perror("accept");
        }

        for (fd_ind = 1; fd_ind < polls.nums; fd_ind++) {
            if (polls.fds[fd_ind].fd != -1) {
                if (polls.fds[fd_ind].revents != 0)
                    dispatch(polls.fds[fd_ind].fd);
                if (polls.fds[fd_ind].revents & (POLLHUP | POLLERR)) {
                    polls.fds[fd_ind].fd = -1;
                    polls.fds[fd_ind].events = 0;
                }                
            }
        }        
    }

    return (void *)0;
}

void * start_gather(int fd)
{
    pthread_t tid;

    if (pthread_create(&tid, NULL, gather_thread, fd)  < 0) {
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
    int reuse = 1;
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

    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    if (ret < 0) {
        perror("Failed to setsockopt");
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

    return fd;

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

void cancel_gather(void *handle)
{
    pthread_t tid;

    if (handle) {
        tid = (pthread_t)handle;
        if (pthread_cancel(tid) < 0) {
            perror("Failed to cancel gather");
        }
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <netinet/in.h>

#include "main.h"
#include "map.h"

int main(int argc, char **argv)
{
    int fd = -1;
    char opt;
    char photon_addr[INET_ADDRSTRLEN] = { 0 };
    short photon_port = -1;
    char other_addr[INET_ADDRSTRLEN] = { 0 };
    short other_port = -1;
    char serv_if[IF_NAMESIZE] = { 0 };
    short serv_port = -1;
    int i;

	for (i = 1; i < argc; i++) {
	   if (strncmp("-P", argv[i], strlen(argv[i])) == 0) {
		   memset(photon_addr, 0, INET_ADDRSTRLEN);
		   if (strlen(argv[i+1]) >= INET_ADDRSTRLEN)
				exit(EXIT_FAILURE);
		   memcpy(photon_addr, argv[i+1], strlen(argv[i+1]));
		   i++;
		   continue;
	   }
	   if (strncmp("-p", argv[i], strlen(argv[i])) == 0) {
		   photon_port = atoi(argv[i+1]);
		   i++;
		   continue;
	   }
	   if (strncmp("-O", argv[i], strlen(argv[i])) == 0) {
		   memset(other_addr, 0, INET_ADDRSTRLEN);
		   if (strlen(argv[i+1]) >= INET_ADDRSTRLEN)
				exit(EXIT_FAILURE);
		   memcpy(other_addr, argv[i+1], strlen(argv[i+1]));
		   i++;
		   continue;
	   }
	   if (strncmp("-o", argv[i], strlen(argv[i])) == 0) {
		   other_port = atoi(argv[i+1]);
		   i++;
		   continue;
	   }
	   if (strncmp("-S", argv[i], strlen(argv[i])) == 0) {
		   memset(serv_if, 0, IF_NAMESIZE);
		   if (strlen(argv[i+1]) >= IF_NAMESIZE)
				exit(EXIT_FAILURE);
		   memcpy(serv_if, argv[i+1], strlen(argv[i+1]));
		   i++;
		   continue;
	   }
	   if (strncmp("-s", argv[i], strlen(argv[i])) == 0) {
		   serv_port = atoi(argv[i+1]);
		   i++;
		   continue;
	   }
   }

   printf("photon_addr=%s; photon_port=%d; other_addr=%s; other_port=%d; serv_if=%s; serv_port=%d;\n",
		   photon_addr, photon_port, other_addr, other_port, serv_if, serv_port);

    if (strlen(photon_addr) == 0 || photon_port <= 0 ||
        strlen(other_addr) == 0 || other_port <= 0 ||
        strlen(serv_if) == 0 || serv_port <= 0
        )
        return 0;

    if (init_mn_fd() < 0)
        return -1;

    fd = init_gather(serv_if, serv_port);
    if (fd < 0) {
        printf("Init gather failed\n");
        exit(EXIT_FAILURE);
    }

    void *handle = NULL;
    handle = start_gather(fd);
    if (!handle) {
        printf("Start gather failed\n");
        deinit_gather(handle, fd);
        exit(EXIT_FAILURE);
    }

    if (init_dispatcher(DISPATCHER_NAME_PHOTON, photon_addr, photon_port) < 0) {
        cancel_gather(handle);
        deinit_gather(handle, fd);
    }

    if (init_dispatcher(DISPATCHER_NAME_OTHER, other_addr, other_port) < 0) {
        cancel_gather(handle);
        deinit_gather(handle, fd);
    }

    deinit_gather(handle, fd);

    return 0;
}
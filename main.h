#ifndef MAIN_H
#define MAIN_H

int init_gather(char *ifname, short port);
void * start_gather(int fd);
void deinit_gather(void *handle, int fd);
void gather_send(int device_fd, const char *buf, int len);
void cancel_gather(void *handle);


#define DISPATCHER_NAME_PHOTON    "photon" 
#define DISPATCHER_NAME_OTHER    "other" 

void dispatcher_send(const char *buf, int len);
int init_dispatcher(const char *name, char *servaddr, short port);

unsigned int CRC16_Checkout (char *puchMsg, int usDataLen);

#endif
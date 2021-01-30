#ifndef MAIN_H
#define MAIN_H

void dispatcher_send(const char *buf, int len);
void gather_send(int device_fd, const char *buf, int len);

unsigned int CRC16_Checkout ( unsigned char *puchMsg, unsigned int usDataLen);

#endif
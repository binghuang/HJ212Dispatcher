#ifndef MAP_H
#define MAP_H

int init_mn_fd(void);
void add_mn_fd(char * mn, int fd);
void del_mn_fd(int fd);
int find_fd(char * mn);
void show_mn_fd(void);


#endif
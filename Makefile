hj212dispatcher: main.c main.h map.c map.h hj212.c hj212.h gather.c dispatcher.c crc16.c
	gcc -o $@ $< -lpthread
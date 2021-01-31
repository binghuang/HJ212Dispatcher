target := hj212dispatcher
obj := main.o map.o hj212.o gather.o dispatcher.o crc16.o
#CFLAGS := -g

$(target): $(obj) 
	gcc $(CFLAGS) -o $@ $(obj) -lpthread

main.o: main.c main.h
	gcc -c $(CFLAGS) $<

map.o: map.c map.h
	gcc -c $(CFLAGS) $<

hj212.o: hj212.c hj212.h
	gcc -c $(CFLAGS) $<

gather.o: gather.c main.h
	gcc -c $(CFLAGS) $<

dispatcher.o: dispatcher.c main.h
	gcc -c $(CFLAGS) $<

crc16.o: crc16.c
	gcc -c $(CFLAGS) $<

.PHONY: clean
clean: 
	@rm -rf $(target) $(obj)
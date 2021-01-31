target := hj212dispatcher
obj := main.o map.o hj212.o gather.o dispatcher.o crc16.o
$(target): $(obj) 
	gcc -o $@ $(obj) -lpthread

main.o: main.c main.h
	gcc -c $<

map.o: map.c map.h
	gcc -c $<

hj212.o: hj212.c hj212.h
	gcc -c $<

gather.o: gather.c main.h
	gcc -c $<

dispatcher.o: dispatcher.c main.h
	gcc -c $<

crc16.o: crc16.c
	gcc -c $<

.PHONY: clean
clean: 
	@rm -rf $(target) $(obj)
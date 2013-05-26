run: sdisk testdisk
	./testdisk sdisk

testdisk: testdisk.o mydisk.o file_system.o
	gcc -o testdisk testdisk.o mydisk.o file_system.o -lm

makedisk: makedisk.o mydisk.o
	gcc -o makedisk makedisk.o mydisk.o

testdisk.o: testdisk.c mydisk.h
	gcc -c testdisk.c

makedisk.o: makedisk.c mydisk.h
	gcc -c makedisk.c

mydisk.o: mydisk.c mydisk.h
	gcc -c mydisk.c
	
file_system.o: file_system.c file_system.h
	gcc -c file_system.c

sdisk: makedisk
	./makedisk sdisk 256

clean: 
	rm -f sdisk *.o makedisk testdisk *~


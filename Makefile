run: sdisk testdisk
	./testdisk sdisk

testdisk: testdisk.o mydisk.o
	gcc -o testdisk testdisk.o mydisk.o

makedisk: makedisk.o mydisk.o
	gcc -o makedisk makedisk.o mydisk.o

testdisk.o: testdisk.c mydisk.h
	gcc -c testdisk.c

makedisk.o: makedisk.c mydisk.h
	gcc -c makedisk.c

mydisk.o: mydisk.c mydisk.h
	gcc -c mydisk.c

sdisk: makedisk
	./makedisk sdisk 256

clean: 
	rm -f sdisk *.o makedisk testdisk *~

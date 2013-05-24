#Simply run 'make' and it will automatically create and format a disk

run: sdisk myformat
	./myformat sdisk

testdisk: testdisk.o mydisk.o
	gcc -o testdisk testdisk.o mydisk.o

makedisk: makedisk.o mydisk.o
	gcc -o makedisk makedisk.o mydisk.o

myformat: myformat.o mydisk.o file_system.o
	gcc -o myformat myformat.o mydisk.o file_system.o

myformat.o: myformat.c file_system.h mydisk.h
	gcc -c myformat.c

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
	rm -f sdisk *.o myformat makedisk testdisk *~

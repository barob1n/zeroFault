


all: zerofault_v2.exe

zerofault_v2.exe: zerofault_v2.o segyIO_class.o
	gcc -O2 -o  zerofault_v2.exe zerofault_v2.o segyIO_class.o -lm 

zerofault_v2.o: zerofault_v2.c
	gcc -O2 -c zerofault_v2.c -lm 

segyIO_class.o: segyIO_class.c
	gcc -O2 -c segyIO_class.c 



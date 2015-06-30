


all: zerofault_v3.exe

zerofault_v3.exe: zerofault_v3.o segyIO_class.o
	gcc -O2 -o  zerofault_v3.exe zerofault_v3.o segyIO_class.o -lm 

zerofault_v3.o: zerofault_v3.c
	gcc -O2 -c zerofault_v3.c -lm 

segyIO_class.o: segyIO_class.c
	gcc -O2 -c segyIO_class.c 



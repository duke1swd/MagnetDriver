all: readpin heartcheck

readpin: readpin.c
	gcc -O readpin.c -lgpiod -o readpin -lwiringPi

heartcheck: heartcheck.c
	gcc -O heartcheck.c -lgpiod -o heartcheck -lwiringPi -lpthread

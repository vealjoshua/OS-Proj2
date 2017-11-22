all: slave master
master: master.c
	gcc -g -o master master.c -I.
slave: slave.c
	gcc -g -o slave slave.c -I.

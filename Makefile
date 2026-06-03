vfs: main filesystem
	gcc -o vfs main.o filesystem.o

main:
	gcc -c main.c -o main.o

filesystem:
	gcc -c filesystem.c -o filesystem.o
vfs: main filesystem
	gcc -g -o vfs main.o filesystem.o -lSDL2main -lSDL2 

main:
	gcc -g -c main.c -o main.o

filesystem:
	gcc -g -c filesystem.c -o filesystem.o
vfs: main filesystem gui
	gcc -g -o vfs main.o filesystem.o gui.o -lSDL2main -lSDL2 -lSDL2_image

main:
	gcc -g -c main.c -o main.o

filesystem:
	gcc -g -c filesystem.c -o filesystem.o

gui:
	gcc -g -c gui.c -o gui.o
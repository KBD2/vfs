vfs: main filesystem gui
	gcc -o vfs main.o filesystem.o gui.o -lSDL2main -lSDL2 -lSDL2_image

main:
	gcc -c main.c -o main.o

filesystem:
	gcc -c filesystem.c -o filesystem.o

gui:
	gcc -c gui.c -o gui.o
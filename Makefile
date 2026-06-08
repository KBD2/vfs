objects = main.o filesystem.o gui.o errors.o

vfs: $(objects)
	gcc -o vfs $(objects) -lSDL2main -lSDL2 -lSDL2_image

.PHONY: clean

clean:
	rm vfs $(objects)
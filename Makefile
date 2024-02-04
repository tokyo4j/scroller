main: main.c
	gcc $(shell pkg-config --cflags --libs libevdev) -g $^ -o $@
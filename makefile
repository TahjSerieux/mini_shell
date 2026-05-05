CC = gcc
CFLAGS = -Wall

run: clean build
	./mini_shell

build: main.c
	@echo "Builing Mini Shell"
	$(CC) $(CFLAGS) main.c -o mini_shell -lreadline

clean:
	rm -f mini_shell
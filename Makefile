CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99


t_editor: t_editor.c
	$(CC) $(CFLAGS) t_editor.c appendBuffer.c errorHandle.c windowSize.c fileIO.c -o t_editor


run: t_editor
	./t_editor
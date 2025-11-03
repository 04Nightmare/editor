t_editor: t_editor.c
	$(CC) t_editor.c appendBuffer.c errorHandle.c windowSize.c -o t_editor -Wall -Wextra -pedantic -std=c99

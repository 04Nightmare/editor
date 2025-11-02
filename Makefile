t_editor: t_editor.c
	$(CC) t_editor.c errorHandle.c -o t_editor -Wall -Wextra -pedantic -std=c99

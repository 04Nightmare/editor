#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<unistd.h>

void err_handle(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s);
	exit(1);
}

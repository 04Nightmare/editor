#ifndef EDITORCONFIG_H
#define EDITORCONFIG_H

#include<termios.h>

typedef struct editorRow {
    int size;
    char *chars;
};

typedef struct editorConfig{
	int cx, cy;
	int screenrows;
	int screencols;
    int numrows;
    struct editorRow row;
	struct termios termi_settings;
};
extern struct editorConfig E;

#endif
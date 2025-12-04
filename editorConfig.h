#ifndef EDITORCONFIG_H
#define EDITORCONFIG_H

#include<termios.h>
#include<time.h>

#define CONFIRM_QUIT 3

typedef struct editorRow {
    int size;
    int rsize;
    char *chars;
    char *render;
}editorRow;

typedef struct editorConfig{
	int cx, cy;
    int rx;
    int rowoffset;
    int coloffset;
	int screenrows;
	int screencols;
    int numrows;
    int changed;
    editorRow *row;
    char *filename;
    char statmessage[110];
    time_t statmessage_time;
	struct termios termi_settings;
};
extern struct editorConfig E;

#endif
#ifndef EDITORCONFIG_H
#define EDITORCONFIG_H

#include<termios.h>
#include<time.h>

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
    editorRow *row;
    char *filename;
    char statmessage[80];
    time_t statmessage_time;
	struct termios termi_settings;
};
extern struct editorConfig E;

#endif
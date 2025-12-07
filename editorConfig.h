#ifndef EDITORCONFIG_H
#define EDITORCONFIG_H

#include<termios.h>
#include<time.h>

#define CONFIRM_QUIT 3
#define CTRL_KEY(k) ((k) & 0x1f)

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

enum editorKey {
	BACKSPACE = 127,
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN,
};

#endif
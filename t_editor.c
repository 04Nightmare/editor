#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<termios.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

#include "editorConfig.h"
#include "errorHandle.h"
#include "windowSize.h"
#include "appendBuffer.h"
#include "fileIO.h"
#include "statusBar.h"

#define EDITOR_VERSION "v1.0"

#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig E;

enum editorKey {
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


void disableRawMode(){
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.termi_settings) == -1)
		err_handle("tcsetattr");
}


void enableRawMode(){
	if (tcgetattr(STDIN_FILENO, &E.termi_settings) == -1) err_handle("tcsetattr");
	atexit(disableRawMode);

	struct termios raw = E.termi_settings;
	raw.c_iflag &= ~(ICRNL | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 5;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) err_handle("tcsetattr");
}


int editorReadKey() {
	int nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if(nread == -1 && errno != EAGAIN) err_handle("read");
	}

	if (c == '\x1b') {
		char seq[3];

		if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
		if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

		if (seq[0] == '[') {
			if (seq[1] >= '0' && seq[1] <= '9') {
				if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
				if (seq[2] == '~'){
					switch (seq[1]) {
						case '1': return HOME_KEY;
						case '3': return DEL_KEY;
						case '4': return END_KEY;
						case '5': return PAGE_UP;
						case '6': return PAGE_DOWN;
						case '7': return HOME_KEY;
						case '8': return END_KEY;
					}
				}
			}else {
				switch (seq[1]) {
					case 'A': return ARROW_UP;
					case 'B': return ARROW_DOWN;
					case 'C': return ARROW_RIGHT;
					case 'D': return ARROW_LEFT;
					case 'H': return HOME_KEY;
					case 'F': return END_KEY;
				} 
			}
		}else if (seq[0] == 'O'){
			switch (seq[1]) {
				case 'H': return HOME_KEY;
				case 'F': return END_KEY;
			}
		}
		return '\x1b';
	}else{
		return c;
	}
}

void editorScroll() {
	E.rx = 0;
	if (E.cy < E.numrows){
		E.rx = CxToRxConvert(&E.row[E.cy], E.cx);
	}

	if (E.cy < E.rowoffset) {
		E.rowoffset = E.cy;
	}
	if (E.cy >= E.rowoffset + E.screenrows) {
		E.rowoffset = E.cy - E.screenrows + 1;
	}
	if (E.rx < E.coloffset) {
		E.coloffset = E.rx;
	}
	if (E.rx >= E.coloffset + E.screencols) {
		E.coloffset = E.rx - E.screencols + 1;
	}
}

void editorDrawRows(struct abuffer *ab) {
	int y;
	for(y = 0; y < E.screenrows; y++){
		int filerow = y + E.rowoffset;
		if (filerow >= E.numrows) {
			if (y == E.screenrows / 3 && E.numrows == 0) {
				char message[80];
				int messagelen = snprintf(message, sizeof(message), "Editor --version %s", EDITOR_VERSION);
				if (messagelen > E.screencols) messagelen = E.screencols;
				int padding = (E.screencols - messagelen) / 2;
				if (padding) {
					appendBuffer(ab, "~", 1);
					padding--;
				}
				while (padding--) appendBuffer(ab, " ", 1);
				appendBuffer(ab, message, messagelen);
			}else{
				appendBuffer(ab, "~", 1);	
			}
		}else {
			int len = E.row[filerow].rsize - E.coloffset;
			if (len < 0) len = 0;
			if (len > E.screencols) len = E.screencols;
			if (len > 0 && E.row[filerow].render){
				appendBuffer(ab, E.row[filerow].render + E.coloffset, len); //or just use a reference like &E.row[filerow].chars[E.coloffset]
			}
		}

		appendBuffer(ab, "\x1b[K", 3);
		appendBuffer(ab, "\r\n", 2);
		//if(y < E.screenrows - 1) {
		//}
	}
}


void editorRefreshScreen() {
	editorScroll();
	struct abuffer ab = ABUFFER_INIT;

	appendBuffer(&ab, "\x1b[?25l", 6);
	appendBuffer(&ab, "\x1b[H", 3);

	editorDrawRows(&ab);
	drawStatusBar(&ab);
	drawMessageInStatbar(&ab);

	char buff[32];
	snprintf(buff, sizeof(buff), "\x1b[%d;%dH", (E.cy - E.rowoffset) + 1, (E.rx - E.coloffset) + 1);
	appendBuffer(&ab, buff, strlen(buff));


	appendBuffer(&ab, "\x1b[?25h", 6);
	write(STDOUT_FILENO, ab.b, ab.len);
	freeBuff(&ab);
}


void editorMoveCursor(int key) {
	editorRow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
	int times = E.screenrows;

	switch (key) {
		case ARROW_LEFT:
			if (E.cx != 0){
				E.cx--;
			}else if (E.cy > 0){
				E.cy--;
				row = &E.row[E.cy];
				E.cx = row->size;
			}
			break;
		case ARROW_RIGHT:
			if (row && E.cx < row->size){
				E.cx++;
			}else if (row && E.cx == row->size){
				E.cy++;
				E.cx=0;
			}
			break;
		case ARROW_UP:
			if (E.cy != 0){
				E.cy--;
			}
			break;
		case ARROW_DOWN:
			if (E.cy < E.numrows-1){
				E.cy++;
			}
			break;
		case PAGE_UP:
			while(times--){
				if (E.cy != 0){
					E.cy--;
				}
			}
			break;
		case PAGE_DOWN:
			while(times--){
				// if (E.cy != E.screenrows - 1){
				 if (E.cy < E.numrows - 1){
					E.cy++;
				}
			}
			break;
		case HOME_KEY:
			E.cx = 0;
			break;
		case END_KEY:
			//row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
			E.cx = row ? row->size : 0;
			break;
	}

	row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
	int rowLength = row ? row->size : 0;
	if (E.cx > rowLength){
		E.cx = rowLength;
	}
}

void editorKeyPress() {
	int c = editorReadKey();
	switch (c){
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
        		write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;

		case HOME_KEY:
			E.cx = 0;
			break;
		case END_KEY:
			if(E.cy < E.numrows)
				E.cx = E.row[E.cy].size;
			break;
		case PAGE_UP:
		case PAGE_DOWN:   //maybe also implement here later for up down.
		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			editorMoveCursor(c);
			break;
	}
}


void initialEditor() {
	E.cx = 0;
	E.cy = 0;
	E.rx = 0;
	E.rowoffset = 0;
	E.coloffset = 0;
	E.numrows = 0;
	E.row = NULL;
	E.filename = NULL;
	E.statmessage[0] = '\0';
	E.statmessage_time = 0;
	if(getWindowSize(&E.screenrows, &E.screencols) == -1) err_handle("getWindowSize");
	E.screenrows -= 2;
}


int main(int argc, char *argv[]) {
	enableRawMode();
	initialEditor();
	if (argc >= 2){
		editorOpen(argv[1]);
	}
	setStatusMessage("HELP: Ctrl-Q = quit");

	while (1){
		editorRefreshScreen();
		editorKeyPress();
	}
	return 0;
}

#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<termios.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>


#include "errorHandle.h"
#include "windowSize.h"
#include "appendBuffer.h"

#define EDITOR_VERSION "v1.0"

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
};


struct editorConfig{
	int cx, cy;
	int screenrows;
	int screencols;
	struct termios termi_settings;
};
struct editorConfig E;


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
			switch (seq[1]) {
				case 'A':
					return ARROW_UP;
				case 'B':
					return ARROW_DOWN;
				case 'C':
					return ARROW_RIGHT;
				case 'D':
					return ARROW_LEFT;
			} 
		}
		return '\x1b';
	}else{
		return c;
	}
}



void editorDrawRows(struct abuffer *ab) {
	int y;
	for(y = 0; y < E.screenrows; y++){
		if (y == E.screenrows / 3) {
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

		appendBuffer(ab, "\x1b[K", 3);
		if(y < E.screenrows - 1) {
			appendBuffer(ab, "\r\n", 2);
		}
	}
}


void editorRefreshScreen() {
	struct abuffer ab = ABUFFER_INIT;

	appendBuffer(&ab, "\x1b[?25l", 6);
	appendBuffer(&ab, "\x1b[H", 3);

	editorDrawRows(&ab);

	char buff[32];
	snprintf(buff, sizeof(buff), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
	appendBuffer(&ab, buff, strlen(buff));
	//appendBuffer(&ab, "\x1b[H", 3);

	appendBuffer(&ab, "\x1b[?25h", 6);
	write(STDOUT_FILENO, ab.b, ab.len);
	freeBuff(&ab);
}


void editorMoveCursor(int key) {
	switch (key) {
		case ARROW_LEFT:
			E.cx--;
			break;
		case ARROW_RIGHT:
			E.cx++;
			break;
		case ARROW_UP:
			E.cy--;
			break;
		case ARROW_DOWN:
			E.cy++;
			break;
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
	if(getWindowSize(&E.screenrows, &E.screencols) == -1) err_handle("getWindowSize");
}


int main(){
	enableRawMode();
	initialEditor();

	while (1){

		editorRefreshScreen();
		editorKeyPress();
	}
	return 0;
}

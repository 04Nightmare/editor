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


#define CTRL_KEY(k) ((k) & 0x1f)


struct editorConfig{
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


char editorReadKey() {
	int nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if(nread == -1 && errno != EAGAIN) err_handle("read");
	}
	return c;
}



void editorDrawRows(struct abuffer *ab) {
	int y;
	for(y = 0; y < E.screenrows; y++){
		appendBuffer(ab, "~", 1);

		if(y < E.screenrows - 1) {
				appendBuffer(ab, "~", 2)
			}
	}
}


void editorRefreshScreen() {
	struct abuffer ab = ABUFFER_INIT;
	appendBuffer(&ab, "\x1b[2J", 4);
	appendBuffer(&ab, "\x1b[H", 3);

	editorDrawRows();
	appendBuffer(&ab, "\x1b[H", 3);
	write(STDOUT_FILENO, ab.b, ab.len);
	freeBuff(&ab);
}



void editorKeyPress() {
	char c = editorReadKey();
	switch (c){
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
        		write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}


void initialEditor() {
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

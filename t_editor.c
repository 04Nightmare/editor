#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<termios.h>
#include<stdlib.h>
#include<ctype.h>

#define CTRL_KEY(k) ((k) & 0x1f)


struct termios termi_settings;


void err_handle(const char *s){
	write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s);
	exit(1);
}


void disableRawMode(){
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termi_settings) == -1)
		err_handle("tcsetattr");
}


void enableRawMode(){
	if (tcgetattr(STDIN_FILENO, &termi_settings) == -1) err_handle("tcsetattr");
	atexit(disableRawMode);

	struct termios raw = termi_settings;
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



void editorRefreshScreen() {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
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


int main(){
	enableRawMode();

	while (1){
		/*char c = '\0';
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) err_handle("read");
		if(iscntrl(c)){
			printf("%d\r\n", c);
		}else{
			printf("%d ('%c')\r\n", c, c);
		}
		if (c == CTRL_KEY('q')) break;
		*/

		editorRefreshScreen();
		editorKeyPress();
	}
	return 0;
}

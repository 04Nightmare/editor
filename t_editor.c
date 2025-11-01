#include<stdio.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<ctype.h>


struct termios termi_settings;


void disableRawMode(){
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &termi_settings);
}



void enableRawMode(){
	tcgetattr(STDIN_FILENO, &termi_settings);
	atexit(disableRawMode);

	struct termios raw = termi_settings;
	raw.c_iflag &= ~(ICRNL | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 5;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main(){
	enableRawMode();

	while (1){
		char c = '\0';
		read(STDIN_FILENO, &c, 1);
		if(iscntrl(c)){
			printf("%d\r\n", c);
		}else{
			printf("%d ('%c')\r\n", c, c);
		}
		if (c == 'q') break;
	}
	return 0;
}

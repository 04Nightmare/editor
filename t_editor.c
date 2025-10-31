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
	raw.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main(){
	enableRawMode();

	char c;
	while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
		if(iscntrl(c)){
			printf("%d\n", c);
		}else{
			printf("%d ('%c')\n", c, c);
		}
	}
	return 0;
}

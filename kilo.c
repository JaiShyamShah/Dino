#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    // Get the original terminal so we can return that to user on exit
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    // Enter raw mode since default is cannonical
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(){
    enableRawMode();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c!='q');
	return 0;
}

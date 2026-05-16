/* includes */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/* data */
 struct termios orig_termios;

/* terminal */
void die(const char *s){
    perror(s);
    exit(1);
}

void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
 }

void enableRawMode() {
    // Get the original terminal so we can return that to user on exit
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcsetattr");
    atexit(disableRawMode);
    // Enter raw mode since default is cannonical
    struct termios raw = orig_termios;
    // Miscellaneous flags BRKINT, INPCK, ISTRIP, and CS8
    // Disable software flow control, carriage return
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    // Disable output processing
    raw.c_oflag &= ~(OPOST);
    // Disable canonical, interupt signals( SIGINT AND SIGTSTP), wait
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cflag &= ~(CS8);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsettr");
 }

/* init */
 int main(){
     enableRawMode();
     while (1) {
         char c = '\0';
         if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
             die("read");
         if (iscntrl(c)) {
             printf("%d\r\n", c);
         } else {
             printf("%d ('%c')\r\n", c, c);
         }
         if ((c == 'q') | (c == 'Q')) break;
     }
     return 0;
 }


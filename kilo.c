/* includes */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
/* defines */
#define CTRL_KEY(k) ((k) & 0x1f)

/* data */
struct editorConfig {
    int screenrows;
    int screencols;
    struct termios orig_termios;
};

struct editorConfig E;
/* terminal */
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");
 }

void enableRawMode() {
    // Get the original terminal so we can return that to user on exit
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcsetattr");
    atexit(disableRawMode);
    // Enter raw mode since default is cannonical
    struct termios raw = E.orig_termios;
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

char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

int getWindowSize(int * rows, int * cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/* append buffer */
struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len +=len;
}

void abFree(struct abuf *ab) {
    free(ab->b);
}

/* output */
void editorDrawRows(struct abuf *ab){
    int y;
    for (y = 0; y < E.screenrows; y++) {
        abAppend(ab, "~", 1);
        // To get tilde on last line
        if (y < E.screenrows -1) {
            abAppend(ab, "\r\n", 2);
        }
    }
}
void editorRefreshScreen(){
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[2J", 4);
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);
    abAppend(&ab, "\x1b[H", 3);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

/* input */
void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

/* init */
void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

 int main(){
     enableRawMode();
     initEditor();
     while (1) {
         editorRefreshScreen();
         editorProcessKeypress();
     }
     return 0;
 }

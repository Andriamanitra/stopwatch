#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>


#define clear_screen() printf("\e[1;1H\e[2J")


struct termios orig_termios;


void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}


void enableRawMode() {
    // disable output line buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    // enable echoing typed characters, and switch to raw (byte-by-byte) mode
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


void hide_cursor() {
    printf("\e[?25l");
}


void show_cursor() {
    printf("\e[?25h");
}


long tdiff_ms(struct timespec *t_start, struct timespec *t_end) {
    long secs = t_end->tv_sec - t_start->tv_sec;
    long nsecs = t_end->tv_nsec - t_start->tv_nsec;
    return 1000 * secs + nsecs / 1000000;
}


void print_header(int running) {
    printf("\e[0;0H"); // move cursor to 0,0
    printf("Stopwatch v0.1");
    printf(" ( ");
    printf("\e[4m%s\e[0m%s", "s", running ? "top " : "tart");
    printf(" | ");
    printf("\e[4m%s\e[0m%s", "r", "eset");
    printf(" | ");
    printf("\e[4m%s\e[0m%s", "q", "uit");
    printf(" ) ");
}


int main() {
    int keypress = 'x';
    hide_cursor();
    atexit(show_cursor);
    enableRawMode();
    clear_screen();
    print_header(0);

    struct pollfd fds = {
        STDIN_FILENO,
        POLLIN,
        0
    };

    int ready;
    int running = 0;
    long elapsed_ms = 0;
    long curr_ms;
    int timeout_ms = 9;
    struct timespec t_start;
    struct timespec t_now;

    while (keypress != 'q') {
        ready = poll(&fds, 1, timeout_ms);
        clock_gettime(CLOCK_MONOTONIC, &t_now);
        if (ready > 0) { // there is data to be read in stdin
            keypress = getchar();
            if (keypress == ' ' || keypress == 's') {
                if (running) {
                    elapsed_ms += tdiff_ms(&t_start, &t_now);
                } else {
                    t_start = t_now;
                }
                running = !running;
                print_header(running);
            } else if (keypress == 'r') {
                running = 0;
                elapsed_ms = 0;
                clock_gettime(CLOCK_MONOTONIC, &t_start);
            }
        } else if (ready == -1) {
            clear_screen();
            printf("Exiting due to error: %m\n");
            return 1;
        }

        if (running) {
            curr_ms = tdiff_ms(&t_start, &t_now);
        } else {
            curr_ms = 0;
        }
        long ms = elapsed_ms + curr_ms;
        long ss = ms / 1000;
        long mm = ss / 60;
        long hh = mm / 60;
        printf("\e7\e[%d;%df%02lu:%02lu:%02lu.%03lu\e8", 2, 2, hh, mm % 60, ss % 60, ms % 1000);
    }
    printf("\e7\e[%d;%dfTotal measured time in milliseconds: %lu\e8\n", 3, 2, elapsed_ms + curr_ms);
    printf("\e[4;0H"); // move cursor to 4,0
    return 0;
}

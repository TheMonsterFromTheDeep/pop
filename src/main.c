#include <ncurses.h>
#include <stdlib.h>

#include <zlib/zentry.h>
#include <zlib/zlist.h>
#include <zlib/zstr.h>

#include "editor.h"

#define CTRL(c) ((c) - 96)

zlist_of(buffer*) buffers;
size_t current_index = 0;
buffer *current = NULL;

void setcurrent(size_t index) {
    if(zsize(buffers) < 1) { return; }
    current = buffers[current_index = index];
}

void render() {
    if(current) {
        buf_render(current);
    }
    else {
        clear();

        size_t w, h;

        #define NONE_OPEN_MSG ("No files are open!")

        getmaxyx(stdscr, h, w);

        mvaddstr(h / 2, w / 2 - (sizeof(NONE_OPEN_MSG) / (2 * sizeof(char))), NONE_OPEN_MSG);
        refresh();
    }
}

void handle(int key) {

    switch(key) {
        case CTRL('n'): {
                buffer *nbuf = buf_new();
                buf_init(nbuf);
                zlist_add(buffers, nbuf);
                setcurrent(zsize(buffers) - 1);
            }
            return;
        case KEY_SLEFT:
            if(current_index > 0) { setcurrent(current_index - 1); }
            return;
        case KEY_SRIGHT:
            if(current_index < zsize(buffers) - 1) { setcurrent(current_index + 1); }
            return;
    }

    if(current) {
        switch(key) {
            case KEY_UP:
                buf_movecursor(current, 0, -1);
                return;
            case KEY_DOWN:
                buf_movecursor(current, 0, 1);
                return;
            case KEY_LEFT:
                buf_movecursor(current, -1, 0);
                return;
            case KEY_RIGHT:
                buf_movecursor(current, 1, 0);
                return;
            case '\b':
            case KEY_BACKSPACE:
            case 127:
                buf_backspace(current);
                return;
            case '\n':
                buf_newline(current);
                return;
            default:
                buf_type(current, key);
        }
    }
    
}

void entry(zargs args) {
    initscr();

    noecho(); /* Set up curses mode */
	cbreak();
    curs_set(0);

    keypad(stdscr, TRUE);

    if(has_colors()) {
        start_color();
    }

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);

    zlist_init(buffers, 0);

    int next = -1;
    
    do {
        handle(next);
        render();
    } while((next = getch()) != CTRL('b'));
    
    size_t i;

    for(i = 0; i < zsize(buffers); ++i) {
        buf_free(buffers[i]);
    }
    zfree(buffers);

    endwin();
}

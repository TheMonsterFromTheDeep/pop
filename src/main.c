#include <ncurses.h>
#include <stdlib.h>

#include <zlib/zentry.h>
#include <zlib/zlist.h>
#include <zlib/zstr.h>

#include "editor.h"

#define CTRL(c) ((c) - 96)

void entry(zargs args) {
    if(args.count < 2) {
        puts("No file specified!");
        return;
    }

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

    buffer *def = buf_new();
    buf_load(def, args.get[1]);
    buf_render(def);

    int next;
    
    while((next = getch()) != CTRL('b')) {
        switch(next) {
            case KEY_UP:
                buf_movecursor(def, 0, -1);
                break;
            case KEY_DOWN:
                buf_movecursor(def, 0, 1);
                break;
            case KEY_LEFT:
                buf_movecursor(def, -1, 0);
                break;
            case KEY_RIGHT:
                buf_movecursor(def, 1, 0);
                break;
            case '\b':
            case KEY_BACKSPACE:
            case 127:
                buf_backspace(def);
                break;
            case '\n':
                buf_newline(def);
                break;
            default:
                buf_type(def, next);
        }
    }
    
    buf_save(def, args.get[1]);
    buf_free(def);

    endwin();
}

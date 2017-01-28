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

enum {
    EDIT,
    NAME
} mode;

void setcurrent(size_t index) {
    if(zsize(buffers) < 1) { return; }
    current = buffers[current_index = index];
}

void render_edit() {
    int w, h;
    getmaxyx(stdscr, h, w);

    size_t i;

    if(current) {
        move(0, 4);        

        for(i = 0; i < zsize(buffers); ++i) {
            if(i == current_index) { attron(A_BOLD); }
            addstr(buffers[i]->name);
            if(i == current_index) { attroff(A_BOLD); }

            if(i < zsize(buffers) - 1) {
                attron(COLOR_PAIR(2));
                addstr(" | ");
                attron(COLOR_PAIR(1));
            }
        }

        move(1, 0);
        attron(COLOR_PAIR(2));
        for(i = 0; i < w; ++i) { addch('-'); }
        attron(COLOR_PAIR(1));

        buf_render(current, 0, 2, w, h - 2);
    }
    else {
        #define NONE_OPEN_MSG ("No files are open!")

        mvaddstr(h / 2 - 1, w / 2 - (sizeof(NONE_OPEN_MSG) / (2 * sizeof(char))), NONE_OPEN_MSG);
        refresh();
    }
}

void render_name() {
    move(0, 0);
    addstr("Enter file path: ");
    addstr(current->path);
}

void render() {
    size_t w, h;
    getmaxyx(stdscr, h, w);

    size_t x, y;
    for(y = 0; y < h; ++y) {
        move(y, 0);
        for(x = 0; x < w; ++x) { addch(' '); }
    }

    switch(mode) {
        case EDIT:
            render_edit();
            break;
        case NAME:
            render_name();
            break;
    }
}

void handle_edit(int key) {
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
            case CTRL('s'):
                buf_save(current, current->path);
                return;
            default:
                if(key >= 32 && key <= 126) {
                    buf_type(current, key);
                }
        }
    }
}

void finalize_buf(buffer *b) {
    buf_load(b, b->path);

    size_t i;

    for(i = zsize(b->path) - 1; i > 0; --i) {
        if(b->path[i] == '/') {
            ++i;
            break;
        }
    }
    b->name = zstr_from(b->path + i);
}

void handle_name(int key) {
    switch(key) {
        case '\b':
        case KEY_BACKSPACE:
        case 127:
            zstr_backspace(current->path);
            return;
        case '\n': {
                finalize_buf(current);

                mode = EDIT;
            }
            return;
        default:
            zstr_catc(&current->path, (char)key);
    }
}

void handle(int key) {
    switch(key) {
        case CTRL('o'): {
                buffer *nbuf = buf_new();
                buf_init(nbuf);
                zlist_add(buffers, nbuf);
                setcurrent(zsize(buffers) - 1);
                mode = NAME;
            }
            return;
        case KEY_SLEFT:
            if(current_index > 0) { setcurrent(current_index - 1); }
            return;
        case KEY_SRIGHT:
            if(current_index < zsize(buffers) - 1) { setcurrent(current_index + 1); }
            return;
    }

    switch(mode) {
        case EDIT:
            handle_edit(key);
            break;
        case NAME:
            handle_name(key);
            break;
    }
}

void entry(zargs args) {
    initscr();

    noecho(); /* Set up curses mode */
	cbreak();
    curs_set(0);

    raw(); /* Helps use extra control keys */

    keypad(stdscr, TRUE);

    if(has_colors()) {
        start_color();
    }

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);

    zlist_init(buffers, 0);

    size_t i;

    for(i = 1; i < args.count; ++i) {
        buffer *nbuf = buf_new();
        buf_init(nbuf);
        zlist_add(buffers, nbuf);

        nbuf->path = zstr_from(args.get[i]);
        finalize_buf(nbuf);
    }

    if(args.count > 1) {
        setcurrent(0);
    }

    mode = EDIT;

    int next = -1;
    
    do {
        handle(next);
        render();
    } while((next = getch()) != CTRL('q'));

    for(i = 0; i < zsize(buffers); ++i) {
        buf_free(buffers[i]);
    }
    zfree(buffers);

    endwin();
}

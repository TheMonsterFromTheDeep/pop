#include <ncurses.h>

#include <zlib/zlist.h>
#include <zlib/zstr.h>

#include "editor.h"

buffer *buf_new() {
    buffer *b = malloc(sizeof(buffer));
    zlist_init(b->lines, 0);
    b->cursor.x = b->cursor.y = 0;

    return b;
}

void buf_free(buffer *b) {
    size_t i;

    for(i = 0; i < zsize(b->lines); ++i) {
        zfree(b->lines[i]);
    }

    zfree(b->lines);

    free(b);
}

void buf_load(buffer *b, const char *path) {
    zlist_of(zstr*) lines = b->lines;

    zlist_clear(lines);

    FILE *f = fopen(path, "r");
    if(f) {
        int next;
        zstr *line = zstr_empty();
        while((next = fgetc(f)) != EOF) {
            if(next == '\n') {
                zlist_add(b->lines, zstr_clone(line));
                zstr_clear(line);
            }
            else {
                zstr_catc(&line, (char)next);
            }
        }
        zlist_add(b->lines, line); //This feels bad but there's technically nothing wrong with it
        fclose(f);
    }
    else { zlist_add(b->lines, zstr_empty()); }
}

void buf_save(buffer *b, const char *path) {
    size_t i;

    FILE *f = fopen(path, "w");
    if(f) {
        for(i = 0; i < zsize(b->lines); ++i) {
            fputs(b->lines[i],f);
            if(i < zsize(b->lines) - 1) { fputc('\n',f); }
        }
        fclose(f);
    }
}

void buf_render(buffer *b) {
    clear();

    size_t i, j, x, line;
    int w, h;

    getmaxyx(stdscr, h, w);

    attron(COLOR_PAIR(2));

    zlist_of(zstr*) lines = b->lines;
    cursor_k cursor = b->cursor;

    for(i = b->start; i < zsize(lines); ++i) {
        line = i + 1;
        for(x = 0; x < 3 && line > 0; ++x) {
            mvaddch(i - b->start, 2 - x, '0' + line % 10);
            line /= 10;
        }
    }

    attron(COLOR_PAIR(1));

    for(i = 0; i < h; ++i) {
        move(i, 4);
        j = 0;
        if(i + b->start < zsize(lines)) {
            zstr *current = lines[i + b->start];

            for(; j < zsize(current); ++j) {
                addch(current[j]);
                if(j >= w - 5) { break; }
            }
        }
        while(j < w - 5) { addch(' '); ++j; }
    }

    attron(A_REVERSE);
    char c = cursor.x < zsize(lines[cursor.y]) ? lines[cursor.y][cursor.x] : ' ';
    mvaddch(cursor.y - b->start, 4 + cursor.x, c);
    attroff(A_REVERSE);

    refresh();
}

void clamp(int *value, int min, int max) {
    if(*value < min) { *value = min; }
    if(*value > max) { *value = max; }
}

void buf_movecursor(buffer *b, int x, int y) {
    y += b->cursor.y;
    x += b->cursor.x;

    clamp(&y, 0, zsize(b->lines) - 1); //It's important to clamp the y first, as it indexes for the x
    clamp(&x, 0, zsize(b->lines[y]));

    size_t w, h;
    getmaxyx(stdscr, h, w);

    if(y >= b->start + h) {
        b->start = y - h + 1;
    }

    if(y < b->start) {
        b->start = y;
    }

    b->cursor.y = y;
    b->cursor.x = x;

    buf_render(b);
}

void buf_newline(buffer *b) {
    zstr *current = b->lines[b->cursor.y];
    zlist_insert(b->lines, b->cursor.y + 1, zstr_copy(current, b->cursor.x, zsize(current)));
    zstr_rewind(current, zsize(current) - b->cursor.x);
    b->cursor.x = 0;
    buf_movecursor(b, 0, 1);
}

void buf_backspace(buffer *b) {
    cursor_k cursor = b->cursor;
    zlist_of(zstr*) lines = b->lines;

    if(cursor.x > 0) {
        zstr_delete(lines[cursor.y], cursor.x - 1);
        buf_movecursor(b, -1, 0);
    }
    else if(cursor.y > 0) {
        b->cursor.x = zsize(lines[cursor.y - 1]);
        zstr_catz(&lines[cursor.y - 1], lines[cursor.y]);
        zfree(lines[cursor.y]);
        zlist_delete(lines, cursor.y);
        buf_movecursor(b, 0, -1);
    }
}

void buf_type(buffer *b, int c) {
    zstr_insertc(&(b->lines[b->cursor.y]), b->cursor.x, (char)c);
    buf_movecursor(b, 1, 0);
}

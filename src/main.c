#include <ncurses.h>
#include <stdlib.h>

#include <zlib/zentry.h>
#include <zlib/zlist.h>
#include <zlib/zstr.h>

zlist_of(zstr*) lines;
size_t start;

struct {
    int x, y;
} cursor;

void generate(const char* base) {
    size_t i;
    size_t start = 0;
    while(base[i]) {
        if(base[i] == '\n') {
            zlist_add(lines, zstr_copy(base, start, i));
            start = i + 1;
        }
        ++i;
    }
    zlist_add(lines, zstr_copy(base, start, i));
}

void load(FILE *f) {
    int next;
    zstr *line = zstr_empty();
    while((next = fgetc(f)) != EOF) {
        if(next == '\n') {
            zlist_add(lines, zstr_clone(line));
            zstr_clear(line);
        }
        else {
            zstr_catc(&line, (char)next);
        }
    }
    zlist_add(lines, line); //This feels bad but there's technically nothing wrong with it
}

void render() {
    clear();

    size_t i, j, x, line;
    int w, h;

    getmaxyx(stdscr, h, w);

    attron(COLOR_PAIR(2));

    for(i = start; i < zsize(lines); ++i) {
        line = i + 1;
        for(x = 0; x < 3 && line > 0; ++x) {
            mvaddch(i - start, 2 - x, '0' + line % 10);
            line /= 10;
        }
    }

    attron(COLOR_PAIR(1));

    for(i = 0; i < h; ++i) {
        move(i, 4);
        j = 0;
        if(i + start < zsize(lines)) {
            zstr *current = lines[i + start];

            for(; j < zsize(current); ++j) {
                addch(current[j]);
                if(j >= w - 5) { break; }
            }
        }
        while(j < w - 5) { addch(' '); ++j; }
    }

    attron(A_REVERSE);
    char c = cursor.x < zsize(lines[cursor.y]) ? lines[cursor.y][cursor.x] : ' ';
    mvaddch(cursor.y - start, 4 + cursor.x, c);
    attroff(A_REVERSE);

    refresh();
}

void clamp(int *value, int min, int max) {
    if(*value < min) { *value = min; }
    if(*value > max) { *value = max; }
}

void movecursor(int x, int y) {
    y += cursor.y;
    x += cursor.x;

    clamp(&y, 0, zsize(lines) - 1); //It's important to clamp the y first, as it indexes for the x
    clamp(&x, 0, zsize(lines[y]));

    size_t w, h;
    getmaxyx(stdscr, h, w);

    if(y >= start + h) {
        start = y - h + 1;
    }

    if(y < start) {
        start = y;
    }

    cursor.y = y;
    cursor.x = x;

    render();
}

void newline() {
    zstr *current = lines[cursor.y];
    zlist_insert(lines, cursor.y + 1, zstr_copy(current, cursor.x, zsize(current)));
    zstr_rewind(current, zsize(current) - cursor.x);
    cursor.x = 0;
    movecursor(0, 1);
}

void backspace() {
    if(cursor.x > 0) {
        zstr_delete(lines[cursor.y], cursor.x - 1);
        movecursor(-1, 0);
    }
    else if(cursor.y > 0) {
        cursor.x = zsize(lines[cursor.y - 1]);
        zstr_catz(&lines[cursor.y - 1], lines[cursor.y]);
        zfree(lines[cursor.y]);
        zlist_delete(lines, cursor.y);
        movecursor(0, -1);
    }
}

void type(int c) {
    zstr_insertc(&lines[cursor.y], cursor.x, (char)c);
    movecursor(1, 0);
}

void entry(zargs args) {
    if(args.count < 2) {
        puts("No file specified!");
        return;
    }

    zlist_init(lines, 0);

    FILE *f = fopen(args.get[1], "r");
    if(f) {
        load(f);
        fclose(f);
    }
    else { zlist_add(lines, zstr_empty()); }

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

    cursor.x = cursor.y = 0;
    render();

    int next;
    
    while((next = getch()) != '\t') {
        switch(next) {
            case KEY_UP:
                movecursor(0, -1);
                break;
            case KEY_DOWN:
                movecursor(0, 1);
                break;
            case KEY_LEFT:
                movecursor(-1, 0);
                break;
            case KEY_RIGHT:
                movecursor(1, 0);
                break;
            case '\b':
            case KEY_BACKSPACE:
            case 127:
                backspace();
                break;
            case '\n':
                newline();
                break;
            default:
                type(next);
        }
    }

    size_t i;

    f = fopen(args.get[1], "w");
    if(f) {
        for(i = 0; i < zsize(lines); ++i) {
            fputs(lines[i],f);
            if(i < zsize(lines) - 1) { fputc('\n',f); }
        }
        fclose(f);
    }
    
    for(i = 0; i < zsize(lines); ++i) {
        zfree(lines[i]);
    }
    zfree(lines);

    endwin();
}

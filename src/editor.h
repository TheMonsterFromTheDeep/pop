#ifndef EDITOR_H
#define EDITOR_H

typedef struct {
    int x, y;
} cursor_k;

typedef struct {
    zstr *name;
    zlist_of(zstr*) lines;
    size_t start;
    cursor_k cursor;
} buffer;

buffer *buf_new(const char *name);
void buf_free(buffer*);

void buf_load(buffer*,const char*);
void buf_save(buffer*,const char*);

void buf_render(buffer*,int,int,int,int);

void buf_movecursor(buffer*,int,int);

void buf_newline(buffer*);
void buf_backspace(buffer*);
void buf_type(buffer*,int);

#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ncurses.h>

#include <zgcl/zlist.h>

#include "file_browser.h"

typedef struct {
	zstr *str;
	unsigned char type;
} entry;



zstr *get_path_at(const char *label, const char *path) {
	DIR *dp;
	struct dirent *ep;

	zlist_of(entry) entries;

	zlist_init(entries, 0);

	clear();
	move(0, 0);

	dp = opendir(path);
	if(dp) {
		while((ep = readdir(dp))) {
			entry e;
			e.str = zstr_from(ep->d_name);
			e.type = ep->d_type;
			zlist_add(entries, e);
		}

		closedir(dp);
	}
	else {
		clear();
		attron(COLOR_PAIR(1));
		addstr("Failed to open file browser!");
		refresh();
		getch();
		return zstr_empty();

	}

	int w, h;
	getmaxyx(stdscr, h, w);

	size_t selected = 0;

	zstr *newFile = zstr_empty();
	zstr *newDir = zstr_empty();
	size_t fCursor = 0;
	size_t dCursor = 0;

	for(;;) {
		move(0, 0);
		attron(COLOR_PAIR(1));
		addstr("    ");
		addstr(path);
		
		attron(COLOR_PAIR(2));
		move(1, 0);
		for(int i = 0; i < w; ++i) { addch(ACS_HLINE); }
		move(h - 2, 0);
		for(int i = 0; i < w; ++i) { addch(ACS_HLINE); }
		move(h - 1, 0);
		for(int i = 0; i < w; ++i) { addch(' '); }
		move(h - 1, 0);
		addstr("    ");
		if(selected < zsize(entries)) {
			if(entries[selected].type == DT_REG) addstr(label);
			else addstr("open directory");
			addch(' ');
			attron(COLOR_PAIR(1));
			addstr(entries[selected].str);
		}
		else if(selected == zsize(entries)) {
			addstr(label);
			addch(' ');

			int x, y;
			getyx(stdscr, y, x);

			attron(COLOR_PAIR(1));
			addstr(newFile);

			attron(A_REVERSE);
			if(fCursor < zsize(newFile)) mvaddch(y, x + fCursor, newFile[fCursor]);
			else addch(' ');
			attroff(A_REVERSE);
		}
		else {
			addstr("create directory ");

			int x, y;
			getyx(stdscr, y, x);

			attron(COLOR_PAIR(1));
			addstr(newDir);

			attron(A_REVERSE);
			if(dCursor < zsize(newDir)) mvaddch(y, x + dCursor, newDir[dCursor]);
			else addch(' ');
			attroff(A_REVERSE);
		}

		if(selected >= zsize(entries)) {
			attron(COLOR_PAIR(2));
			#define CUSTOM_FILE_HELP_MSG " (enter name) "
			move(h - 1, w - (sizeof(CUSTOM_FILE_HELP_MSG) / sizeof(char)) + 1);
			addstr(CUSTOM_FILE_HELP_MSG);
		}

		attron(COLOR_PAIR(1));

		for(size_t i = 0; i < zsize(entries); ++i) {
			move(i + 2, 0);
			addstr("    ");
			if(i == selected) attron(A_REVERSE);
			switch(entries[i].type) {
				case DT_REG: attron(COLOR_PAIR(3)); break;
				case DT_DIR: attron(COLOR_PAIR(4)); break;
				default: attron(COLOR_PAIR(1)); break;
			}
			addstr(entries[i].str);
			attroff(A_REVERSE);
		}

		move(2 + zsize(entries), 0);
		attron(COLOR_PAIR(3));
		addch(' ');
		if(zsize(entries) == selected) attron(A_REVERSE);
		addch(':');
		attroff(A_REVERSE);
		addstr("  ");
		addstr(newFile);
		addstr("  "); /* This takes care of deleted characters */
		if(zsize(entries) == selected) {
			move(2 + zsize(entries), 4 + fCursor);
			attron(A_REVERSE);
			if(fCursor < zsize(newFile)) addch(newFile[fCursor]);
			else addch(' ');
			attroff(A_REVERSE);
		}

		move(2 + zsize(entries) + 1, 0);
		attron(COLOR_PAIR(4));
		addch(' ');
		if(zsize(entries) + 1 == selected) attron(A_REVERSE);
		addch('/');
		attroff(A_REVERSE);
		addstr("  ");
		addstr(newDir);
		addstr("  ");
		if(1 + zsize(entries) == selected) {
			move(2 + zsize(entries) + 1, 4 + dCursor);
			attron(A_REVERSE);
			if(dCursor < zsize(newDir)) addch(newDir[dCursor]);
			else addch(' ');
			attroff(A_REVERSE);
		}

		refresh();

		int next = getch();
		switch(next) {
			case '\n': goto done;

			case KEY_UP:
				if(selected > 0) --selected;
				break;

			case KEY_DOWN:
				if(selected < zsize(entries) + 1) ++selected;
				break;

			case KEY_LEFT:
				if(selected == zsize(entries)) {
					if(fCursor > 0) --fCursor;
				}
				if(selected > zsize(entries)) {
					if(dCursor > 0) --dCursor;
				}
				break;

			case KEY_RIGHT:
				if(selected == zsize(entries)) {
					if(fCursor < zsize(newFile)) ++fCursor;
				}
				if(selected > zsize(entries)) {
					if(dCursor < zsize(newDir)) ++dCursor;
				}
				break;

			case KEY_BACKSPACE:
			case 127:
				if(selected == zsize(entries)) {
					if(fCursor > 0) {
						zstr_delete(newFile, fCursor - 1);
						--fCursor;
					}
				}
				if(selected > zsize(entries)) {
					if(dCursor > 0) {
						zstr_delete(newDir, dCursor - 1);
						--dCursor;
					}
				}
				break;

			default:
				if(next < ' ' || next > 127) break;
				if(selected == zsize(entries)) {
					zstr_insertc(&newFile, fCursor, (char)next);
					++fCursor;
				}
				else if(selected > zsize(entries)) {
					zstr_insertc(&newDir, dCursor, (char)next);
					++dCursor;
				}
		}
	}

done: { }
	zstr *result;

	unsigned char type;

	if(selected < zsize(entries)) {
		result = entries[selected].str;
		type = entries[selected].type;

		zfree(newFile);
		zfree(newDir);
	}
	else if(selected == zsize(entries)) {
		result = newFile;
		type = DT_REG;

		zfree(newDir);
	}
	else {
		mkdir(newDir, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH);

		result = newDir;
		type = DT_DIR;

		zfree(newFile);
	}

	for(size_t i = 0; i < zsize(entries); ++i) {
		if(i != selected) zfree(entries[i].str);
	}
	zfree(entries);

	zstr_inserts(&result, 0, path);

	if(type == DT_REG) {
		return result;
	}
	else {
		zstr_catc(&result, '/');
		zstr *newResult = get_path_at(label, result);
		zfree(result); /* TODO: Don't recurse here */
		return newResult;
	}
}

zstr *open_browser(const char *label) {
	return get_path_at(label, "./");
}

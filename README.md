# pop
pop is a text editor written in curses. As of now it is not particularly complicated.

## building
pop is built off of a custom C library, [zgcl](https://github.com/TheMonsterFromTheDeep/zgcl/). In order to avoid having to install this to /usr/lib, use `make setup` to build a local copy for use by this specific project.

Basically:
```
git clone https://github.com/TheMonsterFromTheDeep/pop
make setup
make
```
## usage
The pop control scheme is very simple. First, to open files directly from the command line, specific them as command line arguments - e.g. `pop file1 file2 "file with spaces"`.

Right now pop supports rudimentary editing - plain and simple manipulation of characters and the cursor.

Controls:
* arrow keys: move cursor
* letters/numbers: insert characters
* backspace: delete characters
* ctrl-s: save
* ctrl-q: quit
* ctrl-o: open a new or existing file
* shift-left-arrow: move to previous file
* shift-right-arrow: move to next file
* ctrl-r: `system()` first line of '.popdebug'

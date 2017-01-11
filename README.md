# pop
pop is a text editor written in curses. As of now it is not particularly complicated.

## building
pop is built off of a custom C library, zLib. In order to avoid having to install this to /usr/lib, use `make setup` to build a local copy for use by this specific project.

Basically:
```
git clone https://github.com/TheMonsterFromTheDeep/pop
make setup
make
```

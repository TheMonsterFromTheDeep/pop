# Executable name
BIN= pop

# Source and build paths
SRC_DIR= ./src
BUILD_DIR= ./build

# Source files
SRCS= main.c

# Object files to build
OBJS= $(SRCS:%.c=%.o)

# Dependencies for each source
DEPS= $(OBJS:%.o=$(BUILD_DIR)/%.d)

# Flags for the compiler
CFLAGS= -Izlib -Lzlib -lncurses -lzlib

# Default path for make install
INSTALL_PATH?=/usr/local

$(BIN) : $(OBJS:%=$(BUILD_DIR)/%)
	$(CC) $^ -o $@ $(CFLAGS)

-include $(DEPS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) -MMD -c $< -o $@ $(CFLAGS)

.PHONY : clean install setup
clean :
	rm -rf $(BUILD_DIR)
	rm $(BIN)

install : $(BIN)
	cp $(BIN) $(INSTALL_PATH)/bin

ZLIB_SRC=https://github.com/TheMonsterFromTheDeep/zlib

setup :
	rm -rf zlib
	mkdir -p zlib
	git clone $(ZLIB_SRC) zlib
	$(MAKE) -C zlib
	cd zlib && cp build/libzlib.a libzlib.a
	$(MAKE) -C zlib clean

desetup :
	rm -rf zlib

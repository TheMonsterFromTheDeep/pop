# Executable name
BIN= pop

# Source and build paths
SRC_DIR= ./src
BUILD_DIR= ./build

# Source files
SRCS= main.c editor.c file_browser.c

# Object files to build
OBJS= $(SRCS:%.c=%.o)

# Dependencies for each source
DEPS= $(OBJS:%.o=$(BUILD_DIR)/%.d)

# Flags for the compiler
CFLAGS= -Wall -Izgcl -Lzgcl -lncurses -lzgcl

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

ZGCL_SRC=https://github.com/TheMonsterFromTheDeep/zgcl

setup :
	rm -rf zgcl
	mkdir -p zgcl
	git clone $(ZGCL_SRC) zgcl
	$(MAKE) -C zgcl
	cd zgcl && cp build/libzgcl.a libzgcl.a
	cd zgcl && mv src zgcl
	$(MAKE) -C zgcl clean

desetup :
	rm -rf zgcl

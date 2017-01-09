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
CFLAGS= -lncurses -lzlib

# Default path for make install
INSTALL_PATH?=/usr/local

$(BIN) : $(OBJS:%=$(BUILD_DIR)/%)
	$(CC) $^ -o $@ $(CFLAGS)

-include $(DEPS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) -MMD -c $< -o $@ $(CFLAGS)

.PHONY : clean install
clean :
	rm -rf $(BUILD_DIR)
	rm $(BIN)

install : $(BIN)
	cp fads $(INSTALL_PATH)/bin

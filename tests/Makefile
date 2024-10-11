NAME := tests
CC := gcc
CC_FLAGS := -g -Wall -Wpedantic -Wextra -Wconversion -O2 # compiler flags
CPP_FLAGS := -MMD -MP -I../ # preprocessor flags
LD_FLAGS := #-lm

BUILD_DIR := ../build/$(NAME)
OBJ_DIR := $(BUILD_DIR)/obj
SRC_DIR := .

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES)) # subsitute .c files into obj path with .o
DEPS := $(OBJS:.o=.d) # substitute .o for .d in all objs

.PHONY: all directories clean
all: directories $(NAME)

directories:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(BUILD_DIR)/$(NAME)

$(NAME): $(OBJS)	
	$(CC) $(OBJS) -o $(BUILD_DIR)/$(NAME) $(LD_FLAGS)

$(OBJ_DIR)/%.o: %.c # can't use $(SOURCE) or $(OBJS) because weird make pattern rules
	$(CC) $(CPP_FLAGS) $(CC_FLAGS) -c $< -o $@

-include $(DEPS)
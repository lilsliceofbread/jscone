NAME := tests
CC := gcc
CC_FLAGS := -g -Wall -Wpedantic -Wextra -Wconversion -O2 # compiler flags
CPP_FLAGS := -MMD -MP -I../ # preprocessor flags
LD_FLAGS := #-lm

IS_WIN=0
ifeq ($(OS),Windows_NT)
	IS_WIN=1
endif

BUILD_DIR := ../build/$(NAME)
OBJ_DIR := $(BUILD_DIR)/obj
SRC_DIR := .

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES)) # subsitute .c files into obj path with .o
DEPS := $(OBJS:.o=.d) # substitute .o for .d in all objs

.PHONY: all directories clean run
all: directories $(NAME)

run: all
	$(BUILD_DIR)/tests

directories:
ifeq ($(IS_WIN),1)
ifeq ("$(wildcard $(BUILD_DIR))","")
	cmd.exe /c mkdir $(subst /,\,$(BUILD_DIR))
endif
ifeq ("$(wildcard $(OBJ_DIR))","")
	cmd.exe /c mkdir $(subst /,\,$(OBJ_DIR))
endif
else
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)
endif

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(BUILD_DIR)/$(NAME)

$(NAME): $(OBJS)	
	$(CC) $(OBJS) -o $(BUILD_DIR)/$(NAME) $(LD_FLAGS)

$(OBJ_DIR)/%.o: %.c # can't use $(SOURCE) or $(OBJS) because weird make pattern rules
	$(CC) $(CPP_FLAGS) $(CC_FLAGS) -c $< -o $@

-include $(DEPS)

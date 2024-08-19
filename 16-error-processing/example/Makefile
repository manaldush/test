NAME := exampleLogger

SRC_DIR     := src
OBJ_DIR     := obj
BUILD_DIR   := build
SRCS        := \
	main.c
SRCS        := $(SRCS:%=$(SRC_DIR)/%)
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

FULL_NAME   := $(BUILD_DIR)/$(NAME)

LIB         := ../build/loggerLib.a

CC          := gcc
CFLAGS      := -Wall -Wextra -Wpedantic -g3 -std=c11 -I../src 

RM          := rm -f

all: $(NAME)

$(NAME): $(OBJS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJS) -L. -l:${LIB} -o $(FULL_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS)-c -o $@ $<

clean:
	$(RM) $(OBJS)
	$(RM) $(FULL_NAME)

.PHONY: clean
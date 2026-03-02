CC := gcc
BUILD_DIR := build
MODE := release

TARGETS := particles
.DEFAULT_GOAL := all
.PHONY: all clean run help list

all: $(TARGETS)

ALL_OBJS :=

particles_CFLAGS := -O3 -march=native -ffast-math -fno-math-errno -std=c99 -Wall -Wextra -Werror -Wformat=2 -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs
particles_CFLAGS += -I./cmd
particles_CFLAGS += -MMD -MP
particles_LDFLAGS := -lXfixes -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

particles_SRCS := \
	main.c

particles_OBJS := $(particles_SRCS:%.c=$(BUILD_DIR)/$(MODE)/particles/%.o)

ALL_OBJS += $(particles_OBJS)

particles: $(particles_OBJS)
	$(CC) $^ -o $@ $(particles_LDFLAGS)

$(BUILD_DIR)/$(MODE)/particles/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(particles_CFLAGS) -c $< -o $@

DEPS := $(ALL_OBJS:.o=.d)

-include $(DEPS)

run: particles
	./particles

clean:
	rm -rf $(BUILD_DIR) $(TARGETS)

list:
	@echo "Available targets:"
	@for t in $(TARGETS); do echo "  $$t"; done

help:
	@echo ""
	@echo "Lathe Makefile"
	@echo ""
	@echo "  make              Build all targets"
	@echo "  make <target>     Build specific binary"
	@echo "  make run          Run first target"
	@echo "  make clean        Remove artifacts"
	@echo "  make list         List targets"
	@echo ""
	@echo "Build specific binary:"
	@echo "  make <target>"
	@echo ""

# ============================================
# Lathe Generated Makefile
# ============================================

CC := gcc
BUILD_DIR := build
MODE := release

CFLAGS := -O3 -march=native -ffast-math -fno-math-errno -std=c99 -Wall -Wextra -Werror -Wformat=2 -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs
CFLAGS += -I./cmd
CFLAGS += -MMD -MP
LDFLAGS := -lXfixes -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGETS := particles
.DEFAULT_GOAL := all

.PHONY: all clean run help list

all: $(TARGETS)

# ============================================
# Source Layout
# ============================================

ALL_OBJS :=

particles_SRCS := \
	main.c

particles_OBJS := $(particles_SRCS:%.c=$(BUILD_DIR)/$(MODE)/%.o)

ALL_OBJS += $(particles_OBJS)

particles: $(particles_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

DEPS := $(ALL_OBJS:.o=.d)

# ============================================
# Compile Rules
# ============================================

$(BUILD_DIR)/$(MODE)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

# ============================================
# Utility Targets
# ============================================

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
	@echo "Targets:"
	@echo "  all        Build all targets (default)"
	@echo "  run        Build and run first target"
	@echo "  clean      Remove build artifacts"
	@echo "  list       List available binaries"
	@echo "  help       Show this help message"
	@echo ""
	@echo "Build specific binary:"
	@echo "  make <target>"
	@echo ""

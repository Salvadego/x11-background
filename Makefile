CC := gcc
CFLAGS := -O3 -std=c99 -Wall -Wextra -Werror -Wformat=2 -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -O3 -march=native -ffast-math -fno-math-errno
CFLAGS += -I./cmd
LDFLAGS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

OBJS := .lathe/release/406e031b8824ea26.o

particles: $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

.lathe/release/406e031b8824ea26.o: main.c
	$(CC) $(CFLAGS) -c main.c -o .lathe/release/406e031b8824ea26.o


CC = gcc
CFLAGS = -Wall -Wextra
OUT = day06

SRCS = $(wildcard *.c)
SRCS += $(wildcard ../common/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

run:
	$(OUT)

clean:
	@rm $(OBJS)
	@rm $(OUT)

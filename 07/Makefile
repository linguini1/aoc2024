CC = gcc
CFLAGS = -Wall -Wextra
OUT = day07

SRCS = $(wildcard *.c)
SRCS += $(wildcard ../common/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT) -lm

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

run:
	$(OUT)

clean:
	@rm $(OBJS)
	@rm $(OUT)

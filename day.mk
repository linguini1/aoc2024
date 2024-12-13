CC = gcc
CFLAGS = -Wall -Wextra
DAY = $(lastword $(subst /, ,$(abspath .)))
OUT = day$(DAY)

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

COMMONDIR = $(abspath ../common)
COMMON_SRCS += $(wildcard $(COMMONDIR)/*.c)
COMMON_OBJS = $(patsubst %.c,%.o,$(COMMON_SRCS))

all: $(OUT)

$(OUT): $(OBJS) $(COMMON_OBJS)
	$(CC) $(OBJS) $(COMMON_OBJS) -o $(OUT) $(LINK_FLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

run: $(OUT)
	@echo "Output for Day $(DAY)"
	$(abspath $(OUT)) input.txt

clean:
	@rm $(OBJS)
	@rm $(OUT)

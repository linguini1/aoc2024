CC = gcc
CFLAGS = -Wall -Wextra

YEAR = 2024
DAY = $(lastword $(subst /, ,$(abspath .)))

OUT = day$(DAY)
INPUT = input.txt

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

COMMONDIR = $(abspath ../common)
COMMON_SRCS += $(wildcard $(COMMONDIR)/*.c)
COMMON_OBJS = $(patsubst %.c,%.o,$(COMMON_SRCS))

run: QUIET = 1
ifndef QUIET
.SILENT:
endif

all: $(OUT)

$(OUT): $(OBJS) $(COMMON_OBJS)
	$(CC) $(OBJS) $(COMMON_OBJS) -o $(OUT) $(LINK_FLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

run: $(OUT) $(INPUT)
	@echo "Output for Day $(DAY)"
	$(abspath $(OUT)) input.txt

input: $(INPUT)

$(INPUT):
	python3 ../getinput.py $(YEAR) $(DAY) > $(INPUT)

clean:
	@rm $(OBJS)
	@rm $(OUT)

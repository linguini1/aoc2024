CC = gcc
CFLAGS = -Wall -Wextra
OUT = day$(DAY)

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

COMMON_SRCS += $(wildcard ../common/*.c)
COMMON_OBJS = $(patsubst %.c,%.o,$(COMMON_SRCS))

all: $(OUT)

$(OUT): $(OBJS) $(COMMON_OBJS)
	$(CC) $(OBJS) $(COMMON_OBJS) -o $(OUT) $(LINK_FLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

run:
	$(OUT)

clean:
	@rm $(OBJS)
	@rm $(OUT)

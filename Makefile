CC=gcc

BIN    = m3u8_downloader

SRCS := $(filter-out $(EXCLUDED), $(wildcard *.c))
OBJS   = $(patsubst %.c,%.o,$(SRCS))

CFLAGS:= -g -Wall
LIBS:= -lcurl

CFLAGS+=-O2

all: $(BIN)

$(BIN): $(SRCS:.c=.o)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN) $(LIBS)

.PHONY : clean
clean:
	@rm -f $(BIN)
	@rm -f $(SRCS:.c=.o)

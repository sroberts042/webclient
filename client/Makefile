CC = g++
DEBUG_FLAGS = -g -O0 -DDEBUG
CFLAGS = $(DEBUG_FLAGS) -Wall
RM = rm -f

all: tcp_client

web_client: tcp_client.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	$(RM) *.o tcp_client

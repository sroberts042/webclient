CC = gcc
DEBUG_FLAGS = -g -O0 -DDEBUG
CFLAGS = $(DEBUG_FLAGS) -Wall
RM = rm -f

all: tcp

servers: tcp_server udp_server

clients: tcp_client udp_client

tcp: tcp_server 

udp: udp_server udp_client

tcp_server: tcp_server.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

tcp_client: tcp_client.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

udp_server: udp_server.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

udp_client: udp_client.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	$(RM) *.o tcp_server tcp_client udp_server udp_client

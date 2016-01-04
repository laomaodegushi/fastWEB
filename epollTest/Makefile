all:epoll_server client
.PHONY:all

epoll_server:epoll_server.o
	gcc -o $@ $^
client:client.o
	gcc -o $@ $^
..c.o:
	gcc -c $<

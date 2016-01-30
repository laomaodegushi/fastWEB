#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <fcntl.h> 
#include <string.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#define SERVER_PORT  9090

#define MAXEPOLLSIZE 10000
#define MAXBUF 1024

/************************************************************************/
/* set fd non-blocking 
 return : succees 0
/************************************************************************/
int setnonblocking(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);

    if (flags==-1) {
        perror("fcntl(sock, F_GETFL)");
        return -1;
    }
    flags |= O_NONBLOCK;
    if (-1 == fcntl(sock, F_SETFL, flags)) {
        perror("fcntl(sock, F_SETFT, flags)");
        return -2;
    }
    return 0;
}
/************************************************************************/
/* main                                                                     */
/************************************************************************/
int main(int argc, char **argv)
{
    int backlog = 1024;//max connect num 

    int n,nread,serverfd, newclifd, epfds, nfds, maxfds;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    int socket_size = sizeof(struct sockaddr_in);

    struct epoll_event ev;
    struct epoll_event events[MAXEPOLLSIZE];
    char buf[MAXBUF];
    
    //create server socket
    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("can't create server socket");
        exit(1);
    }
    // set socket nonblocking
    if (setnonblocking(serverfd) < 0) {
        perror("setnonblock error");
    }
    //local information
    memset((void *)&server_addr,0,socket_size);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    //bind
    if (bind(serverfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind error");
        exit(1);
    }
    //listen 
    if (listen(serverfd, backlog) == -1) {
        perror("listen error");
        exit(1);
    }
    //create epoll and add
    epfds = epoll_create(MAXEPOLLSIZE);

    ev.data.fd = serverfd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfds, EPOLL_CTL_ADD, serverfd, &ev);
    maxfds = 1;
    printf("epoll-server starts up\nServer port: %d, max fd is %d, backlog is %d\n", SERVER_PORT, MAXEPOLLSIZE, backlog);

    while (1){
        //wait fd event
        nfds = epoll_wait(epfds, events, maxfds, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            continue;
        }
        //process the fds-event
        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == serverfd) {//new client socket connect
                if ((newclifd = accept(serverfd, (struct sockaddr *)&client_addr, &socket_size)) ==-1) {
                    perror("accept error");
                    continue;
                }
                if (maxfds >= MAXEPOLLSIZE) {
                    fprintf(stderr, "too many fds, more than %d", MAXEPOLLSIZE);
                    close(newclifd);
                    continue;
                }
                printf("Server get connection with %s\n\a", inet_ntoa(client_addr.sin_addr));
                // set new client socket nonblocking
                if (setnonblocking(newclifd) < 0) {
                    perror("set newclifd nonblocking error");
                }
                //add newfd to epfds
                ev.data.fd = newclifd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfds, EPOLL_CTL_ADD, newclifd, &ev);
                maxfds++;
                continue;
            }
            //fei lianjie chuli
            if (events[n].events & EPOLLIN) {//in ready
                if ((nread = recv(events[n].data.fd, buf, MAXBUF,0))==-1) { //read client socket stream
                    perror("read error or client close the connection");
                    close(events[n].data.fd);
                }
                buf[nread]='\0';
                printf("clientMSG : %s", buf);
                ev.data.fd=events[n].data.fd;
                ev.events = EPOLLOUT | EPOLLET;
                epoll_ctl(epfds, EPOLL_CTL_MOD, events[n].data.fd, &ev);
            } else if (events[n].events & EPOLLOUT) {//out ready
        	char* ackbuf = "I am the server!\n";
                send(events[n].data.fd, ackbuf, strlen(ackbuf),0); 
                close(events[n].data.fd);
		        ev.data.fd = events[n].data.fd;
                ev.events = EPOLLOUT | EPOLLET;
                epoll_ctl(epfds, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                maxfds--;
            }
        }
    }
    close(serverfd);
    return 0;
}

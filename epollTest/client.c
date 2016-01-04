#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  SIN_PORT 9090
char* askbuf="Who are you?\n";
int main(int argc, char *argv[])
{
    int sockfd;
    char buffer[1024];
    struct sockaddr_in server_addr;
    struct hostent *host;
    int nbytes;

    if (argc != 2) {
        fprintf(stdout, "Usage:%s hostname \a\n", argv[0]);
        exit(1);
    }

    if ((host = gethostbyname(argv[1])) == NULL) {
        fprintf(stdout, "Gethostname error\n");
        exit(1);
    }

    /* create sockfd */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stdout, "Socket Error:%s\a\n", strerror(errno));
        exit(1);
    }

    /* local socket */
    memset((void *) &server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SIN_PORT);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);

    /* connect server */
    if (connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1) {
        fprintf(stdout, "Connect Error:%s\a\n", strerror(errno));
        exit(1);
    }

    /* succees  */
    if (send(sockfd, askbuf, strlen(askbuf),0) == -1) {
        fprintf(stdout, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    printf("Send to Server:%s", askbuf);
    if ((nbytes = recv(sockfd, buffer,sizeof(buffer) ,0)) == -1) {
        fprintf(stdout, "Read Error:%s\n", strerror(errno));
        exit(1);
    }
    buffer[nbytes] = '\0';
    printf("Server Response:%s", buffer);
    /* close */
    close(sockfd);
    exit(0);
}

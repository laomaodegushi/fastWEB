#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


//stl head

#include <ext/hash_map> //包含hash_map 的头文件

//#include //stl的map

using namespace std; //std 命名空间

using namespace __gnu_cxx; //而hash_map是在__gnu_cxx的命名空间里的



int init_thread_pool(int threadNum);
void *epoll_loop(void* para);
void *check_connect_timeout(void* para);


struct sockStruct
{
    time_t time;

    unsigned int* recvBuf;
};

//hash-map

//hash_map        sock_map;

hash_map<int, sockStruct>        sock_map;

 
#define MAXRECVBUF 4096
#define MAXBUF MAXRECVBUF+10 

int fd_Setnonblocking(int fd)
{
    int op;
 
    op=fcntl(fd,F_GETFL,0);
    fcntl(fd,F_SETFL,op|O_NONBLOCK);
 
    return op;
}
 
void on_sigint(int signal)
{
    exit(0);
}
 
/* 
handle_message - 处理每个 socket 上的消息收发 
*/ 
int handle_message(int new_fd) 
{ 
    char buf[MAXBUF + 1]; 
    char sendbuf[MAXBUF+1]; 
    int len; 
    /* 开始处理每个新连接上的数据收发 */ 
    bzero(buf, MAXBUF + 1); 
    /* 接收客户端的消息 */ 
    //len = recv(new_fd, buf, MAXBUF, 0);



    int nRecvBuf = MAXRECVBUF; //设置为32K 

    setsockopt(new_fd, SOL_SOCKET, SO_RCVBUF, ( const char* )&nRecvBuf, sizeof(int));
    len=recv(new_fd,&buf, MAXBUF,0);

    //--------------------------------------------------------------------------------------------

    //这块为了使用ab测试

    char bufSend[1000] = {0};
    sprintf(bufSend,"HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n%s","Hello world!\n");
    send(new_fd,bufSend,strlen(buf),0);

    //--------------------------------------------------------------------------------------------


    if (len > 0){ 

        //printf ("%d接收消息成功:'%s'，共%d个字节的数据\n", new_fd, buf, len); 


        //hash-map

        
        hash_map<int, sockStruct>::iterator it_find;
        it_find = sock_map.find(new_fd);
        if(it_find == sock_map.end()){
            //新的网络连接，申请新的接收缓冲区，并放入map中

            //printf("new socket %d\n", new_fd);


            sockStruct newSockStruct;
            newSockStruct.time = time((time_t*)0);
            newSockStruct.recvBuf = (unsigned int*)malloc(1000);
            memset(newSockStruct.recvBuf, 0, 1000);
            strcat((char*)newSockStruct.recvBuf, buf);
            sock_map.insert(pair<int,sockStruct>(new_fd, newSockStruct));
        }else{
            //网络连接已经存在，找到对应的数据缓冲区，将接收到的数据拼接到数据缓冲区中

            //printf("socket %d exist!\n", it_find->first);


            (it_find->second).time = time((time_t*)0);                //时间更改

            char* bufSockMap = (char*)(it_find->second).recvBuf;    //数据存储


            strcat(bufSockMap, buf);
            //printf("bufSockMap:%s\n", bufSockMap);

        }


    } 
    else { 
        if (len < 0) 
            printf ("消息接收失败！错误代码是%d，错误信息是'%s'\n", 
            errno, strerror(errno)); 
        else {
            //将socket从map中移除

            /*
            hash_map::iterator it_find;
            it_find = sock_map.find(new_fd);
            sock_map.erase(it_find);
            */
            printf("client %d quit!\n",new_fd); 
        }
        //close(new_fd); 

        return -1; 
    } 
    /* 处理每个新连接上的数据收发结束 */ 

    //关闭socket的时候，要释放接收缓冲区。

    hash_map<int, sockStruct>::iterator it_find;
    it_find = sock_map.find(new_fd);
    free((it_find->second).recvBuf);
    sock_map.erase(it_find);

    close(new_fd);
    return len; 
} 


    int listenfd;
    int sock_op=1;
    struct sockaddr_in address;
    struct epoll_event event;
    struct epoll_event events[1024];
    int epfd;
    int n;
    int i;
    char buf[512];
    int off;
    int result;
    char *p;

int main(int argc,char* argv[])
{

    init_thread_pool(1);

    signal(SIGPIPE,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGINT,&on_sigint);
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&sock_op,sizeof(sock_op));
 
    memset(&address,0,sizeof(address));
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=htons(8006);
    bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    listen(listenfd,1024);
    fd_Setnonblocking(listenfd);
 
    epfd=epoll_create(65535);
    memset(&event,0,sizeof(event));
    event.data.fd=listenfd;
    event.events=EPOLLIN|EPOLLET;
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&event);

    while(1){
        sleep(1000);
    }
    return 0;
}

/*************************************************
* Function: * init_thread_pool
* Description: * 初始化线程
* Input: * threadNum:用于处理epoll的线程数
* Output: * 
* Others: * 此函数为静态static函数,
*************************************************/
int init_thread_pool(int threadNum)
{
    int i,ret;

    pthread_t threadId;

    //初始化epoll线程池

    for ( i = 0; i < threadNum; i++)
    {

        ret = pthread_create(&threadId, 0, epoll_loop, (void *)0);
        if (ret != 0)
        {
            printf("pthread create failed!\n");
            return(-1);
        }
    }

    ret = pthread_create(&threadId, 0, check_connect_timeout, (void *)0);

    return(0);
}
/*************************************************
* Function: * epoll_loop
* Description: * epoll检测循环
* Input: * 
* Output: * 
* Others: * 
*************************************************/
static int count111 = 0;
static time_t oldtime = 0, nowtime = 0;
void *epoll_loop(void* para)
{
        while(1)
    {
        n=epoll_wait(epfd,events,4096,-1);
        //printf("n = %d\n", n);

        if(n>0)
        {
            for(i=0;i<n;++i)
            {
                if(events[i].data.fd==listenfd)
                {
                    while(1)
                    {
                        event.data.fd=accept(listenfd,NULL,NULL);
                        if(event.data.fd>0)
                        {
                            fd_Setnonblocking(event.data.fd);
                            event.events=EPOLLIN|EPOLLET;
                            epoll_ctl(epfd,EPOLL_CTL_ADD,event.data.fd,&event);
                        }
                        else
                        {
                            if(errno==EAGAIN)
                            break;
                        }
                    }
                }
                else
                {
                    if(events[i].events&EPOLLIN)
                    {
                        //handle_message(events[i].data.fd);


                        char recvBuf[1024] = {0}; 

                        int ret = 999;

                        int rs = 1;


                        while(rs)
                        {
                            ret = recv(events[n].data.fd,recvBuf,1024,0);// 接受客户端消息

                            if(ret < 0)
                            {
                                //由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可//读在这里就当作是该次事件已处理过。

                                if(errno == EAGAIN)
                                {
                                    printf("EAGAIN\n");
                                    break;
                                }
                                else{
                                    printf("recv error!\n");
                                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &event);
                                    close(events[i].data.fd);
                                    break;
                                }
                            }
                            else if(ret == 0)
                            {
                                // 这里表示对端的socket已正常关闭. 

                                rs = 0;
                            }
                            if(ret == sizeof(recvBuf))
                                rs = 1; // 需要再次读取

                            else
                                rs = 0;
                        }




                        if(ret>0){

                            count111 ++;



                            struct tm *today;
                            time_t ltime;
                            time( &nowtime );

                            if(nowtime != oldtime){
                                printf("%d\n", count111);
                                oldtime = nowtime;
                                count111 = 0;
                            }


                            char buf[1000] = {0};
                            sprintf(buf,"HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n%s","Hello world!\n");
                            send(events[i].data.fd,buf,strlen(buf),0);


                            //    CGelsServer Gelsserver;

                            //    Gelsserver.handle_message(events[i].data.fd);

                        }


                        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &event);
                        close(events[i].data.fd);

                    }
                    else if(events[i].events&EPOLLOUT)
                    {
                        sprintf(buf,"HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n%s","Hello world!\n");
                        send(events[i].data.fd,buf,strlen(buf),0);
                        /*
                        if(p!=NULL)
                        {
                            free(p);
                            p=NULL;
                        }
                        */
                        close(events[i].data.fd);
                    }
                    else
                    {
                        close(events[i].data.fd);
                    }
                }
            }
        }
    }

}
/*************************************************
* Function: * check_connect_timeout
* Description: * 检测长时间没反应的网络连接，并关闭删除
* Input: * 
* Output: * 
* Others: * 
*************************************************/
void *check_connect_timeout(void* para)
{
    hash_map<int, sockStruct>::iterator it_find;
    for(it_find = sock_map.begin(); it_find!=sock_map.end(); ++it_find){
        if( time((time_t*)0) - (it_find->second).time > 120){                //时间更改


            free((it_find->second).recvBuf);
            sock_map.erase(it_find);

            close(it_find->first);
        }
    }

}

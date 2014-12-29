/*************************************************************************
  > File Name: client.c
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Mon 29 Dec 2014 05:21:27 PM CST
 ************************************************************************/

#include<stdio.h>

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<syslog.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

#include <string>
#include <iostream>

#define BUFSIZE 1024

int main(void)
{
    int sockfd;
    struct sockaddr_in server_addr;
    struct sockaddr_in clientB, myInfo;
    char buffer[BUFSIZE];
    int server_ip;
    int nrecv;
    socklen_t alen;

    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(5666);
    //inet_pton(AF_INET,"127.0.0.1",(void*)&server_ip);
    inet_pton(AF_INET,"172.19.103.71",(void*)&server_ip);
    server_addr.sin_addr.s_addr=server_ip;

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        syslog(LOG_ERR,"ruptime_udp:socket() failed!\n");
        exit(1);
    }

    memset(buffer,0,BUFSIZE);
    buffer[0]=0;
    alen=sizeof(struct sockaddr);
    if(sendto(sockfd,buffer,1,0,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))<0){
        syslog(LOG_ERR,"ruptime_udp error:%s\n",strerror(errno));
        exit(1);
    }
    if((nrecv=recvfrom(sockfd,buffer,BUFSIZE,0,(struct sockaddr*)&server_addr,&alen))>=0) {
        if (nrecv != sizeof(clientB) * 2) {
            printf("addr error, return, %d, %d\n", sizeof(clientB), nrecv);
            return 0;
        }

        memcpy((char *)&clientB, buffer, sizeof(clientB));
        std::string rip = inet_ntoa(clientB.sin_addr);
        short rport = ntohs(clientB.sin_port);
        printf("remote addr:%s:%u\n", rip.c_str(), (unsigned short)rport);

        memcpy((char *)&myInfo, buffer + sizeof(myInfo), sizeof(myInfo));
        rip = inet_ntoa(myInfo.sin_addr);
        rport = ntohs(myInfo.sin_port);
        printf("myInfo: addr:%s:%u\n", rip.c_str(), (unsigned short)rport);

        sendto(sockfd, buffer, 1, 0, (struct sockaddr *)&clientB, sizeof(sockaddr));
        if((nrecv = recvfrom(sockfd, buffer, BUFSIZE, 0 ,(struct sockaddr*)&server_addr, &alen))>=0) {
        }
    }

    std::string send;
    while (true) {
        std::cin >> send;
        if (!send.find("quit", 0) != std::string::npos) {
            sendto(sockfd, send.c_str(), send.size(), 0, (struct sockaddr *)&clientB, sizeof(sockaddr));
        } else {
            printf("quit...\n");
            return 0;
        }

        memset(buffer, 0, 100);
        if((nrecv = recvfrom(sockfd, buffer, BUFSIZE, 0 ,(struct sockaddr*)&server_addr, &alen))>=0) {
            std::string rip = inet_ntoa(server_addr.sin_addr);
            short rport = ntohs(server_addr.sin_port);
            printf("recv len:%d\n", nrecv);
            printf("remote addr:%s:%u:%s\n", rip.c_str(), (unsigned short)rport, buffer);
        }
    }

    exit(0);
}

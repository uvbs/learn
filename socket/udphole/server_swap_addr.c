/*************************************************************************
  > File Name: server_swap_addr.c
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Mon 29 Dec 2014 06:56:35 PM CST
 ************************************************************************/

#include<stdio.h>

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>

#include <string>

#define BUFSIZE 1024

int main(void)
{
    struct sockaddr_in local_addr,remote_addr1, remote_addr2, remote_addr;
    int sockfd;
    int server_ip,nrecv;
    socklen_t alen = sizeof(remote_addr);
    FILE *fp;
    char buffer[BUFSIZE];
    int count = 0;

    memset(&local_addr,0,sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(5666);
    //inet_pton(AF_INET,"127.0.0.1",(void*)&server_ip);
    inet_pton(AF_INET,"172.19.103.71",(void*)&server_ip);
    local_addr.sin_addr.s_addr=server_ip;

    sockfd =socket(AF_INET,SOCK_DGRAM,0);
    if(bind(sockfd,(struct sockaddr*)&local_addr,sizeof(struct sockaddr))<0){
        fprintf(stderr,"error:bind() error!\n");
        exit(-1);
    }

    printf("====================Begin to service====================\n");
    while(1){
        if((nrecv=recvfrom(sockfd, buffer,BUFSIZE,0,(struct sockaddr*)&remote_addr,&alen))<0){
            fprintf(stderr,"error:%s\n",strerror(errno));
            exit(1);
        }else{
            std::string rip = inet_ntoa(remote_addr.sin_addr);
            short rport = ntohs(remote_addr.sin_port);
            printf("remote addr:%s:%u\n", rip.c_str(), (unsigned short)rport);

            count ++;
            if (count == 1) {
                remote_addr1 = remote_addr;
                continue;
            } else if (count == 2) {
                remote_addr2 = remote_addr;
            }

            printf("send:%d\n", sizeof(remote_addr2) * 2);
            memcpy(buffer, (char *)&remote_addr2, sizeof(remote_addr2));
            memcpy(buffer + sizeof(remote_addr1), (char *)&remote_addr1, sizeof(remote_addr1));
            sendto(sockfd, buffer, sizeof(remote_addr2) * 2, 0, (struct sockaddr*)&remote_addr1,alen); 

            memcpy(buffer, (char *)&remote_addr1, sizeof(remote_addr1));
            memcpy(buffer + sizeof(remote_addr2), (char *)&remote_addr2, sizeof(remote_addr1));
            sendto(sockfd, buffer, sizeof(remote_addr1) * 2, 0, (struct sockaddr*)&remote_addr2,alen); 

            break; 
        }
    }
    printf("end...\n");
    exit(0);
}

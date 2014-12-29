/*************************************************************************
  > File Name: server.c
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Mon 29 Dec 2014 04:32:08 PM CST
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
	struct sockaddr_in local_addr,remote_addr;
	int sockfd;
	int server_ip,nrecv;
	socklen_t alen = sizeof(remote_addr);
	FILE *fp;
	char buffer[BUFSIZE];

	memset(&local_addr,0,sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(5666);
	//inet_pton(AF_INET,"127.0.0.1",(void*)&server_ip);
	inet_pton(AF_INET,"192.168.216.132",(void*)&server_ip);
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
		    sendto(sockfd,buffer,strlen(buffer),0,(struct sockaddr*)&remote_addr,alen); 
            std::string rip = inet_ntoa(remote_addr.sin_addr);
            short rport = ntohs(remote_addr.sin_port);
            printf("remote addr:%s:%u\n", rip.c_str(), (unsigned short)rport);
		}
        usleep(100000); //100ms

	}
	exit(0);
}

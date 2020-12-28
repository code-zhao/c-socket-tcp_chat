 ///
 /// @file    server.cc
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2020-12-24 16:47:30
 ///
 
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>
#include <string>

using namespace std;

#define ERROR_EXIT(msg)do{\
	perror(msg);\
	exit(EXIT_FAILURE);\
}while(0)

pthread_mutex_t mutex1;
pthread_cond_t cond1;
bool flag = true;
string sig;
void *recvMsg(void *arg)
{
	pthread_mutex_lock(&mutex1);
	while(!flag)
	{
		pthread_cond_wait(&cond1, &mutex1);
	}
	int new_fd = *(int*)arg;
	char buff[1024] = {0};
	cout << "waiting for msg..." << endl; 
	int ret = recv(new_fd, buff, sizeof(buff), 0);
	if(ret < 0)
	{
		if(-1 == ret && errno == EINTR)
			return nullptr;
		else
			perror("recv");
	}
	else if(0 == ret)
	{
		close(new_fd);
	}
	else
	{
		cout << "receive from client: " << buff << endl;
	}
	if(flag == true)
	{
		flag = false;
	}
	pthread_mutex_unlock(&mutex1);
	pthread_cond_signal(&cond1);
	return nullptr;
}

void *sendMsg(void *arg)
{
	pthread_mutex_lock(&mutex1);
	while(flag)
	{
		pthread_cond_wait(&cond1, &mutex1);
	}
	int new_fd = *(int*)arg;
	char buff[1024] = {0};
	cout << "waiting for ur input: ";
	cin >> buff;
	sig = buff;
	if(-1 == send(new_fd, buff, strlen(buff), 0))
	{
		close(new_fd);
		perror("send");
	}
	if(flag == false)
		flag = true;
	pthread_mutex_unlock(&mutex1);
	pthread_cond_signal(&cond1);
	return nullptr;
}

void server()
{
	//socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1)
	{
		ERROR_EXIT("socket");
	}

	int reuse = 1;
	if(-1 == setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)))
	{
		ERROR_EXIT("setsockport");
	}
	if(-1 == setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int)))
	{
		ERROR_EXIT("setsockport");
	}

	//bind
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(struct sockaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8888);
	serveraddr.sin_addr.s_addr = inet_addr("192.168.221.168");
	if(-1 == bind(sfd, (const sockaddr*)&serveraddr, sizeof(serveraddr)))
	{
		close(sfd);
		ERROR_EXIT("bind");
	}

	//listen
	if(-1 == listen(sfd, 10))
	{
		close(sfd);
		ERROR_EXIT("listen");
	}
	cout << "server is listening, ready to accept a new link" << endl;
	
	//accept
	struct sockaddr_in clientaddr;
	memset(&clientaddr, 0, sizeof(struct sockaddr));
	socklen_t addrlen = sizeof(struct sockaddr);
	int new_fd = accept(sfd, (struct sockaddr*)&clientaddr, &addrlen);
	if(-1 == new_fd)
	{
		close(sfd);
		ERROR_EXIT("accept");
	}

	printf("success connect--->client ip:%s port:%d\n", inet_ntoa(clientaddr.sin_addr), 
			ntohs(clientaddr.sin_port));

	while(sig != "q")
	{
		flag = true;
		pthread_mutex_init(&mutex1, NULL);
		pthread_cond_init(&cond1, NULL);

		pthread_t recvid;
		pthread_t sendid;
		if(-1 ==pthread_create(&recvid, nullptr, recvMsg, (void*)&new_fd))
		{
			ERROR_EXIT("pthread_create");
		}
		if(-1 == pthread_create(&sendid, nullptr, sendMsg, (void*)&new_fd))
		{
			ERROR_EXIT("pthread_create");
		}
		pthread_join(recvid, nullptr);
		pthread_join (sendid, nullptr);
		
		pthread_mutex_destroy(&mutex1);
		pthread_cond_destroy(&cond1);
	}	
	close(sfd);
}

void print()
{
	puts("-----------server----------");
	puts("---------------------------");
	puts("       c++ tcp chat!       ");
	puts("---------------------------");
	puts("press 'q' to quit when the");
	puts("statement changes to input.");
	puts("---------------------------");
}

int main(void)
{
	print();
	server();
	return 0;
}

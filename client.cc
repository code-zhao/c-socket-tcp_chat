 ///
 /// @file    client.cc
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2020-12-24 18:24:00
 ///

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <string.h>
#include <pthread.h>

using namespace std; 

#define ERROR_EXIT(msg)do{\
	perror(msg);\
	exit(-1);\
}while(0)

pthread_mutex_t mutex1;
pthread_cond_t cond1;
bool flag;
string sig;

void *sendMsg(void *arg)
{
	pthread_mutex_lock(&mutex1);
	while(!flag)
	{
		pthread_cond_wait(&cond1, &mutex1);
	}
	int cfd = *(int*)arg;
	char buff[1024] = {0};
	cout << "waiting for ur input: ";
	cin >> buff;
	sig = buff;
	if(-1 == send(cfd, buff, strlen(buff), 0))
	{
		close(cfd);
		perror("send");
	}
	if(flag)
	{
		flag = false;
	}
	pthread_mutex_unlock(&mutex1);
	pthread_cond_signal(&cond1);
	return nullptr;
}

void *recvMsg(void *arg)
{
	pthread_mutex_lock(&mutex1);
	while(flag)
	{
		pthread_cond_wait(&cond1, &mutex1);
	}
	int cfd = *(int*)arg;
	char buff[1024] = {0};
	cout << "waiting for msg..." << endl; 
	int ret = recv(cfd, buff, sizeof(buff), 0);
	if(ret < 0)
	{
		if(-1 == ret && errno == EINTR)
			return nullptr;
		else
			perror("recv");
	}
	else if(0 == ret)
	{
		close(cfd);
	}
	else
	{
		cout << "receive from server: " << buff << endl;
	}
	if(flag)
		flag = false;
	pthread_mutex_unlock(&mutex1);
	pthread_cond_signal(&cond1);
	return nullptr;
}
void client()
{
	//socket
	int cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(cfd == -1)
	{
		ERROR_EXIT("socket");
	}

	//connect
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8888);
	serveraddr.sin_addr.s_addr = inet_addr("192.168.221.168");

	if(-1 == connect(cfd, (const struct sockaddr*)&serveraddr, 
			sizeof(serveraddr)))
	{
		ERROR_EXIT("connect");
		close(cfd);
	}

	cout << "client and server has been connected!" << endl;
	while(sig != "q")
	{
		flag = true;
		pthread_mutex_init(&mutex1, NULL);
		pthread_cond_init(&cond1, NULL);
		pthread_t sendid;
		pthread_t recvid;
		//send
		if(-1 ==  pthread_create(&sendid, nullptr, sendMsg, (void*)&cfd))
		{
			ERROR_EXIT("pthread_create");
		}
		//recv
		if(-1 == pthread_create(&recvid, nullptr, recvMsg, (void*)&cfd))
		{
			ERROR_EXIT("pthread_create");
		}
		pthread_join(sendid, nullptr);
		pthread_join(recvid, nullptr);
	}//end of while

	close(cfd);
}
void print()
{
	puts("-----------client----------");
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
	client();
	return 0; 
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
void *do_chat(void *); //채팅 메세지를 보내는 함수
int pushClient(int); //새로운 클라이언트가 접속했을 때 클라이언트 정보 추가
int popClient(int); //클라이언트가 종료했을 때 클라이언트 정보 삭제
pthread_t thread;
pthread_mutex_t mutex;
#define MAX_CLIENT 10
#define CHATDATA 1024
#define INVALID_SOCK -1
#define PORT 9000
struct u{
	int list_c; //접속한 클라이언트를 관리하는 배열
	char list_cname[CHATDATA];
};
struct u user[MAX_CLIENT];
char    escape[ ] = "exit";
char    greeting[ ] = "Welcome to chatting room\n";
char    CODE200[ ] = "Sorry No More Connection\n";
int main(int argc, char *argv[ ])
{
    int c_socket, s_socket;
    struct sockaddr_in s_addr, c_addr;
    int    len;
    int    i, j, n;
    int    res;
    if(pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Can not create mutex\n");
        return -1;
   	 }
    s_socket = socket(PF_INET, SOCK_STREAM, 0);
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    if(bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1) {
        printf("Can not Bind\n");
        return -1;
    	}
    if(listen(s_socket, MAX_CLIENT) == -1) {
        printf("listen Fail\n");
        return -1;
    	}
    for(i = 0; i < MAX_CLIENT; i++)
        user[i].list_c = INVALID_SOCK;
    while(1) {
        len = sizeof(c_addr);
        c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
        res = pushClient(c_socket); //접속한 클라이언트를 list_c에 추가
        if(res < 0) { //MAX_CLIENT만큼 이미 클라이언트가 접속해 있다면,
            write(c_socket, CODE200, strlen(CODE200));
            close(c_socket);
        } else {
            write(c_socket, greeting, strlen(greeting));
			//pthread_create width do_chat function.
            pthread_create(&thread,NULL,do_chat,(void *)&c_socket);
        	}
    	}
}
void *do_chat(void *arg)
{
	int c_socket = *((int *)arg);
	char chatData[CHATDATA];
	char tempData[CHATDATA];
	int i, n;
	char *ptr, *whi, *send, *recive;

	while(1) {
		memset(chatData, 0, sizeof(chatData));
		if((n = read(c_socket, chatData, sizeof(chatData))) > 0) {
			//write chatData to all clients
			strcpy(tempData, chatData); //chatData를 tempData로 복사
			ptr = strstr(tempData,"/w"); //tempData에서 /w가 있는지 확인하는 변수
			if(ptr==NULL){ //귓속말이 없으면
				for(i=0;i<MAX_CLIENT;i++){//유저 전체에게 chatData를 보낸다.
					if(user[i].list_c != INVALID_SOCK && user[i].list_c != c_socket) 
						write(user[i].list_c,chatData,n);
				}
				if(strstr(chatData,escape) != NULL){//chatData에 escape문자가 있으면
					popClient(c_socket); //내보내기
					break;
				}
			}
			else{ //귓속말을 했을 때
				recive = strtok(chatData," ");
				whi = strtok(NULL,"/w ");
				send = strtok(NULL," ");
				for(i=0;i<MAX_CLIENT;i++){
					if(strcmp(user[i].list_cname,whi)==0){ //귓속말을 전할 유저의 아이디를 비교
						write(user[i].list_c,chatData,strlen(chatData));
						write(user[i].list_c,send,strlen(send));
					}
				}
			}		
		}
	}
}
int pushClient(int c_socket) {
	//ADD c_socket to list_c array.
	//return -1, if list_c is full.
	//return the index of list_c which c_socket is added.
	int i,n;
	char cuser[50];

	memset(cuser,0,sizeof(cuser));
	
	for(i=0;i<MAX_CLIENT;i++){
		pthread_mutex_lock(&mutex);//user[i].list_c가 겹치지 않도록 락을 걸어둔다.
		if(user[i].list_c == INVALID_SOCK) {
			user[i].list_c = c_socket;
			if((n=read(c_socket,cuser,sizeof(cuser))) > 0){//정상적으로 실행 됐을 때
				strcpy(user[i].list_cname,cuser);//유저의 닉네임 설정
			}
			pthread_mutex_unlock(&mutex);//락을 풀어줌
			return i;
		}
		pthread_mutex_unlock(&mutex);
	}
	if(i == MAX_CLIENT)//닉네임이 겹칠경우
		return -1;
}
int popClient(int c_socket)
{
	int i;
    close(c_socket);//종료
	//REMOVE c_socket from list_c array.
    for(i=0;i<MAX_CLIENT;i++){
		pthread_mutex_lock(&mutex);
		if(c_socket == user[i].list_c){
			user[i].list_c = INVALID_SOCK;
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
	 }
	return 0;
}

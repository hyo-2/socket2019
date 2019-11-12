#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 10000
#define BUFSIZE 10000
char buffer[BUFSIZE];
char rcvBuffer[BUFSIZE];
int numClient = 0;

void do_service(int c_socket);
void sig_handler();

int main(){
	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	int len;
	int n;
	int pid;
	signal(SIGCHLD, sig_handler); //자식 프로세스가 죽었을 때 sig_handler실행
	//첫번째 인자: 시그널 번호, 두번째 인자: 첫번째 인자의 시그널이 발생했을 때 실행되는 함수 명


	s_socket = socket(PF_INET, SOCK_STREAM, 0);

	memset(&s_addr, 0, sizeof(s_addr)); 
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);

	if(bind(s_socket,(struct sockaddr *) &s_addr, sizeof(s_addr)) == -1){ 
		printf("Cannot Bind\n"); 
		return -1;
	}

	if(listen(s_socket, 5) == -1){
		printf("listen Fail\n");
		return -1;
	}

	while(1){ 
		len = sizeof(c_addr);
		printf("클라이언트 접속을 기다리는 중....\n");
		c_socket = accept(s_socket, (struct sockaddr *)&c_addr, &len);
		printf("/client is connectied\n");
		numClient++;
		printf("현재 접속중인 클라이언트 수 : %d\n", numClient);
 
		if((pid=fork())>0){ //부모
			continue;
		} else if(pid == 0){ //자식
			do_service(c_socket);
			numClient--;
			exit(0);
		} else { //fork()함수 실패
			printf("fork() failed\n");
			exit(0);
		}
		printf("/client is connected\n");
		printf("클라이언트 접속 허용\n");
		do_service(c_socket);
	}
	close(c_socket);
	return 0;	
}

void do_service(int c_socket){
	int n;
		while(1){
			n = read(c_socket, rcvBuffer, sizeof(rcvBuffer));
			printf("rcvBuffer: %s\n", rcvBuffer);

			if(strncasecmp(rcvBuffer, "quit", 4) == 0 || strncasecmp(rcvBuffer, "kill server", 11) == 0)
				break;
			
			else if(!strncmp(rcvBuffer, "안녕하세요", strlen("안녕하세요")))
				strcpy(buffer, "안녕하세요. 만나서 반가워요.");

			else if(!strncmp(rcvBuffer, "이름이 머야?", strlen("이름이 머야?")))
				strcpy(buffer, "내 이름은 이효은.");

			else if(!strncmp(rcvBuffer, "몇살이야?", strlen("몇살이야?")))
				strcpy(buffer, "나는 22살이야.");

			else if(!strncasecmp(rcvBuffer, "strlen ", 7) & strlen(rcvBuffer) > 7)
				sprintf(buffer, "문자열의 길이는 %d.", strlen(rcvBuffer) -7 );

			else if(!strncasecmp(rcvBuffer, "strcmp ", 7)){
				char *token;
				char *str[3];
				int i = 0;
				token = strtok(rcvBuffer, " ");
				while(token != NULL){
					str[i++] = token;
					token = strtok(NULL, " ");
				}
				if(i < 3)
					sprintf(buffer, "비교하려면 두 문자열이 필요합니다.");
				else if(!strcmp(str[1], str[2])) 	
					sprintf(buffer, "%s와 %s는 같은 문자열이다.", str[1], str[2]);
				else 
					sprintf(buffer, "%s와 %s는 다른 문자열이다.", str[1], str[2]);

			}
			else
				strcpy(buffer, "못 알아듣겠습니다.");
			n = strlen(buffer);
			write(c_socket, buffer, n);
		}
}

void sig_handler(int signum){
	int pid;
	int status;
	pid = wait(&status);
	printf("pid[%d] is terminated. status = %d\n", pid, status);
	numClient--;
	printf("현재 접속중인 클라이언틑 수: %d\n", numClient);
}

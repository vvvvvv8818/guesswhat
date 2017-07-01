#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "keyword.h"

#define PORT 7777
#define BUF_SIZE 1000
#define ID_LEN 20
#define GAME_NUM 5


struct player{
	int fd;				//소켓 디스크립터
	int score;			//점수
	char id[ID_LEN];	//아이디
	int pipes[2];		//파이프
}typedef player;
player players[4];	//접속한 플레이어 리스트
int num_players;	//접속한 플레이어 수

int fdc[2];	//모든 프로세스 -> read server
int fds[2];	//read server -> accept server

FILE * file;
int log_file;
char tmp_log[50];

void start_game(char * buf);
int send_keyword();
void server_socket(int * serv_sock, struct sockaddr_in * serv_addr);
int distin_msg(char * buf, char * id, char * msg);
void connected_read(int clnt_sock);
void connected_write(int clnt_sock);
int id_exist(char * input, int len);
void new_player(int clnt_sock, char * buf, int str_len);
void notice_new();
void send_players(int clnt_sock);
int my_write(int sock, void* buf, int signedLen);
int my_read(int fd, void * buf, int buf_size);
void sigchld_handling(int sig);
void sigint_handling(int sig);
void error_handling(char * msg);
void write_log(int num, int pid, char * msg);

int main(int argc, char *argv[]){
	file = fopen("log.txt", "w");
	log_file = fileno(file);
	write(log_file, "번호/pid/설명\n", 14);
	printf("Start\n");

	int pid;
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	char buf[BUF_SIZE];
	memset(&buf, 0, BUF_SIZE);
	memset(&players, 0, sizeof(players));

	pipe(fdc);
	pipe(fds);

	int i;
	for(i = 0; i<4; i++)
		pipe(players[i].pipes);


	/* signal handling */
	struct sigaction act;
	act.sa_handler = sigchld_handling;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGCHLD, &act, 0);

	struct sigaction act2;
	act2.sa_handler = sigint_handling;
	sigemptyset(&act2.sa_mask);
	act2.sa_flags = 0;
	sigaction(SIGINT, &act2, 0);

	/* socket(), bind()*/
	server_socket(&serv_sock, &serv_addr);
	/* listen() */
	if( listen(serv_sock, 15) < 0 )
		error_handling("listen() error");

	/* 1번 fork() : 연결, 채팅 프로세스 분리 */
	write_log(0, getpid(), "First fork()");
	pid = fork();
	int childPid = pid;

	// read server(child process)
	if(pid == 0){
		close(serv_sock);
		close(clnt_sock);

		write_log(1, getpid(), "This is read server");
		while(1){
			int res;
			memset(&buf, 0, BUF_SIZE);
			res = my_read(fdc[0], buf, BUF_SIZE);
			if( res == -1 )
				error_handling("read server my_read error");
			else if( res == 0 )
			{
				printf("read server return : 1st read == 0\n");//연결종료
				return 0;
			}
			buf[strlen(buf)] = '\0';

			printf("1) read server : %s\n", buf);

			if( strcmp(buf, "^") == 0 )
			{		//timeout. keyword 담당 서버에 알리기
				my_read(fdc[0], buf, BUF_SIZE);
				my_read(fdc[0], buf, BUF_SIZE);
				my_read(fdc[0], buf, BUF_SIZE);
				my_write(fds[1], "^", 1);
			}
			else
			{
				int i;
				for(i = 0; i < 4; i++)
					my_write(players[i].pipes[1], buf, res);
				my_write(fds[1], buf, res);
			}
			write_log(1, getpid(), buf);
			if( strncmp(buf, "~", 1) == 0 )
			{
				printf("read server ~\n");
				write_log(1, getpid(), "return read server");
				return 0;
			}
		}
	}
	// accept server (parent process)
	else{
		write_log(0, getpid(), "This is parent process.");
		for(num_players = 0; num_players < 4; )
		{
			//printf("2) num_players : %d\n", num_players);

			sprintf(tmp_log, "num_players : %d", num_players);
			write_log(0, getpid(), tmp_log);

			/* accept() */
			int clnt_addr_len = sizeof(clnt_addr);
			clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
			if(clnt_sock == -1)
			{
				error_handling("accept() error");
			}

			/* ID 입력받기 ^_T */
			while(1)
			{
				int str_len;
				memset(&buf, 0, BUF_SIZE);
				str_len = my_read(clnt_sock, buf, BUF_SIZE);
				if(str_len == -1)
					error_handling("id error");////ㅌ??꺼지면안돼...
				if( id_exist(buf, str_len) == -1 )//중복검사
				{
					new_player(clnt_sock, buf, str_len);
					int res = 1;
					my_write(clnt_sock, &res, sizeof(res));
					send_players(clnt_sock);
					notice_new();
					str_len = my_read(fds[0], buf, BUF_SIZE);
					buf[str_len] = '\0';
					printf("login success : %s\n", buf);

					sprintf(tmp_log, "Login successed. (%d)", clnt_sock);
					write_log(0, getpid(), tmp_log);
					break;
				}
				else
				{
					//로그인 실패 메시지 보내기
					printf("login is failed\n");
					int res = -1;
					my_write(clnt_sock, &res, sizeof(res));
					write_log(0, getpid(), "Login failed.");
					continue;
				}
			}//ID입력받기 끝

			/* 2번 fork() : connected process */
			int pid = fork();
			if(pid == 0)//connected
			{
				//printf("3) 3번째 fork()\n");
				write_log(0, getpid(), "This is connected process.");
				close(serv_sock);
				/* 3번 fork() : 입출력 프로세스 분리 */
				pid = fork();
				if(pid == 0)//read담당
				{
					write_log(2, getpid(), "This is connected read process.");
					connected_read(clnt_sock);
					break;
				}
				else//write담당
				{
					write_log(3, getpid(), "This is connected write process.");
					connected_write(clnt_sock);
					break;
				}
				printf("3번 fork() 이후 return(2번 fork()의 자식 프로세스)\n");
				return 0;
			}
			else if(pid == -1)
			{
				error_handling("fork() error");
			}
			else//accept server
			{
				//close(clnt_sock);
				if( num_players == 4 )
				{
					write_log(0, getpid(), "Game start");
					printf("!!!!!!!!!! full\n");
					sleep(3);
					start_game(buf);
					printf("~~ end start_game \n");
					write_log(0, getpid(), "game is finished");
					fclose(file);
					return 0;
				}
				continue;
			}
		}//for loop end
		int i;
		for(i = 0; i< 4; i++)
		{
			close(players[i].fd);
		}
	}
	printf("main return\n");
	write_log(0, getpid(), "finished");
	fclose(file);
	return 0;
}


/* 게임을 진행한다. GAME_NUM만큼 진행 하면서
 * 키워드(문제)를 랜덤으로 정해 출제자(drawer)의 아이디와 함께 전송하고
 * 플레이어들로부터 오는 메시지를 정답과 비교하여 정답처리를 한다.
 * 매개변수
 *  char * buf : 버퍼
 * 메시지구조
 *  새로운 문제 시작됨을 알림 : "*"
 *  클라이언트로부터 오는 채팅 메시지 : "[id] msg"
 *  전송할 문제 : "<drawer_id> keyword"
 *  정답 알림 : "<player_id> correct!" 
 * */
void start_game(char * buf)
{
	printf("[ start_game() ]\n");
	int current_keyword;
	int game_cnt;
	int drawer;		//current drawer index
	int is_selected[KEYWORD_NUM];	//문제 중복출제 되지 않도록 확인
	memset(is_selected, 0, sizeof(is_selected));

	//GUME_NUM만큼 게임 진행(출제)
	for(game_cnt = 0; game_cnt < GAME_NUM; game_cnt++)
	{
		char full [1] = {'*'};
		my_write(fdc[1], full, 1);

		drawer = game_cnt%4;
		printf("@ drawer : %d\n", drawer);

		write_log(0, getpid(), full);
		sprintf(tmp_log, "game count : %d", game_cnt);
		write_log(0, getpid(), tmp_log);				
		sprintf(tmp_log, "drawer : %d", drawer);
		write_log(0, getpid(), tmp_log);

		int i = 0;
		for(i = 5; i >= 1; i-- )
		{
			printf("%d\n", i);
			sleep(1);
		}

		current_keyword = send_keyword(is_selected, drawer);
		//pipe에서 읽어서 확인
		while(1)
		{
			memset(buf, 0, BUF_SIZE);
			int res = my_read(fds[0], buf, BUF_SIZE);
			write_log(0, getpid(), buf);
			printf("### res : %d\n", res);
			if( res == 0)
			{
				continue;//EOF //계속 진행
			}
			else if( res == -1 )
				error_handling("keyword checking error\n");
			if( !strcmp(buf, "^") )
			{
				printf("# timeout");
				write_log(0, getpid(), "timeout");
				break; //while loop 빠져나감
			}
			else
			{
				buf[res] = '\0';
				char id[ID_LEN];
				char msg[BUF_SIZE];
				int res = distin_msg(buf, id, msg);
				if( res != -1 )
				{
					printf("@ id/msg : %s/%s\n", id, msg);
					if( strcmp(KEYWORD[current_keyword], msg) == 0 )
					{
						memset(buf, 0, BUF_SIZE);
						strcat(buf, "<");
						strcat(buf, id);
						strcat(buf, ">");
						strcat(buf, " correct!");
						my_write(fdc[1], buf, strlen(buf));
						printf("%s\n", buf);
						write_log(0, getpid(), buf);
						break;	//정답을 맞춰야만 while문 빠져나옴
					}
				}
			}//if-else end
		}//while end
		printf("~ to next keyword..\n");
	}//for end
	char * end_msg = "~ 게임 끝 ~";
	my_write(fdc[1], end_msg, strlen(end_msg));
	printf("end of game_start()\n");
}



/* 키워드(문제)를 랜덤으로 선택해서 파이프로 보내는 함수
 * 매개변수
 *  int is_selected[] : 선택되었던 키워드 인덱스를 구분하는 배열
 *  int drawer : 현재 출제자의 players 배열에서의 인덱스
 * 리턴값
 *  선택된 키워드의 인덱스
 * 메시지 구조 : "<id of drawer> keyword" 
 * */
int send_keyword(int is_selected[], int drawer)
{
	printf("[ send_keyword() ]\n");
	/* 문제 출제 : "<drawer id> keyword" */
	//read server에 write
	int key_index;
	while(1)
	{
		key_index =	select_keyword();
		if( is_selected[key_index] == 0 )
		{
			is_selected[key_index] = 1;
			break;
		}
	}
	printf("key_index : %d\n", key_index);
	//?悶??현재 ㅓ誰╂?붙여서 write 
	char * key_msg = malloc( strlen(players[drawer].id) + strlen(KEYWORD[key_index]) +3);
	strcpy(key_msg, "<");
	strcat(key_msg, players[drawer].id);
	strcat(key_msg, ">");
	strcat(key_msg, KEYWORD[key_index]);
	printf("key_msg : %s\n", key_msg);
	my_write(fdc[1], key_msg, strlen(key_msg));

	sprintf(tmp_log, "send_keyword() : %s", key_msg);
	write_log(0, getpid(), tmp_log);
	return key_index;
}


/* 클라이언트로부터 온 데이터를 ID와 메시지로 구분하여 저장
 * 매개변수
 *  char buf[] : 클라이언트로부터 온 데이터
 *  char * id : ID를 저장할 포인터
 *  char * msg : 메시지를 저장할 포인터
 * 리턴값
 *  성공시 1, 클라이언트로부터 온 메시지가 아닐 경우 -1 리턴.
 * 메시지구조
 *  클라이언트로부터 온 데이터 : "[id] msg"
 * */
int distin_msg(char buf[], char * id, char * msg)
{
	printf("[ distin_msg() ] buf : %s\n", buf);
	if( *buf == '[' )
	{
		strtok(buf, " ");
		strcpy(id, strtok(NULL, " "));
		//printf("id : %s/", id);

		strtok(NULL, " ");
		char * tmp = strtok( NULL, "" );
		strcpy(msg, tmp);
		//printf("msg : %s\n", msg);
	}
	else
	{
		printf("this msg is not from client!\n");
		write_log(0, getpid(), "distin_msg() : This message is not from client.");
		return -1;
	}
	sprintf(tmp_log, "distin_msg() : %s/%s", id, msg);
	write_log(0, getpid(), tmp_log);
	return 1;
}


/* 서버 소켓 생성을 위한 socket(), bind() 과정을 처리하는 함수 
 * 매개변수
 *  int * serv_sock : 서버 소켓 디스크립터 저장할 변수
 *  struct sockaddr_in * serv_addr : 서버 소켓 주소정보 저장할 변수
 *  */
void server_socket(int * serv_sock, struct sockaddr_in * serv_addr)
{	
	/* sock() */
	*serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( *serv_sock == -1)
		error_handling("socket() error");

	int sockopt = 1;
	setsockopt(*serv_sock, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));

	/* bind() */
	memset(serv_addr, 0, sizeof(*serv_addr));
	(*serv_addr).sin_family = AF_INET;
	(*serv_addr).sin_addr.s_addr = htonl(INADDR_ANY);
	(*serv_addr).sin_port = htons(PORT);
	if( bind(*serv_sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) == -1 )
		error_handling("bind() error");
}


/* 클라이언트와 연결된 커넥티드 소켓을 담당하는 프로세스에서 호출하는 함수.
 * 클라이언트로부터 메시지를 읽어와 파이프로 전송한다.
 * 매개변수
 *  int clnt_sock : 클라이언트 소켓 디스크립터
 * */
void connected_read(int clnt_sock)
{
	printf("[ connected_read() ]\n");

	//to server
	char message[BUF_SIZE];

	while(1){
		memset(&message, 0, BUF_SIZE);
		int signedLen = my_read(clnt_sock, message, BUF_SIZE);
		if( signedLen == 0)
		{
			printf("# read == 0\n");
			break;
		}
		message[strlen(message)] = '\0';					
		my_write(fdc[1], message, signedLen);
		write_log(2, getpid(), message);
	}
	printf("end of conneted_read (%d)\n", clnt_sock);	
}

/* 클라이언트와 연결된 커넥티드 소켓을 담당하는 프로세스에서 호출하는 함수.
 * 서버(파이프)로부터 메시지를 읽어 클라이언트에게 전송한다.
 * 매개변수
 *  int clnt_sock : 클라이언트 소켓 디스크립터
 * */
void connected_write(int clnt_sock)
{
	printf("[ connected_write() ]\n");
	int my_num = num_players -1;
	char message[BUF_SIZE];

	read(players[my_num].pipes[0], message, sizeof(message));

	while(1){
		memset(&message, 0, sizeof(message));
		int signedLen = my_read(players[my_num].pipes[0], message, sizeof(message));
		message[strlen(message)] = '\0';
		//printf("%d child to client : %s\n", client, message);
		my_write(clnt_sock, message, signedLen);
		write_log(3, getpid(), message);

		if( strncmp( message, "~", 1) == 0 )
		{

			write_log(3, getpid(), "end of connected_write");
			break;
		}
	}
}


/* 매개변수로 들어온 아이디가 중복되는지 확인 
 * 매개변수
 *  char * input : 아이디
 *  int len : input의 길이
 * 리턴값
 *  중복되면 중복되는 플레이어의 인덱스, 중복되지 않으면 -1을 리턴
 * */
int id_exist(char*input, int len)
{
	int i;
	for(i = 0; i<num_players; i++)
	{
		//change
		if( strcmp(players[i].id, input) == 0 )
			return i;
	}
	return -1;
}



/* 플레이어 배열(players)에 새로운 플레이어를 추가하는 함수 
 * num_players값이 1 증가함 
 * 매개변수
 *  int clnt_sock : 새로운 클라이언트 소켓 디스크립터
 *  char * buf : 새로운 클라이언트 아이디가 담긴 버퍼
 *  int str_len : 버퍼 길이
 * */
void new_player(int clnt_sock, char * buf, int str_len)
{
	printf("[ new_player() ]\n");
	players[num_players].fd = clnt_sock;
	players[num_players].score = 0;
	memset(players[num_players].id, 0, ID_LEN);
	strncpy(players[num_players].id, buf, str_len);
	printf("fd : %d / score : %d\n", players[num_players].fd, players[num_players].score);
	printf("id : %s\n", players[num_players].id);

	num_players++;
	sprintf(tmp_log, "[ new_player() ] : %d/%s", players[num_players].fd, players[num_players].id);
	//printf("new_player() / num_players++ : %d\n", num_players);
}


/* 새로운 플레이어(가장 최근에 저장된 플레이어)의 id를 read server에게 전송
 * read server에서 새로운 플레이어가 접속했음을 퉝layers 배열의 모든 플에이어에게 알린다 
 * 메시지구조
 *  "'new_player_id'"
 * */
void notice_new()
{
	printf("[ notice_new() ] / %s \n", players[num_players-1].id);

	// to read server pipe
	char * msg = malloc(strlen(players[num_players-1].id) +3);
	memset(msg, 0, sizeof(msg));
	strncat(msg, "'", 1);
	strncat(msg, players[num_players-1].id, strlen(players[num_players-1].id));
	strncat(msg, "'", 1);
	msg[strlen(msg)] = '\0';

	printf("* msg : %s\n", msg);
	my_write(fdc[1], msg, strlen(msg));
	write_log(0, getpid(), msg);
	free(msg);
}


/* 새로운 플레이어에게 접속되어 있는 플레이어의 아이디를 전송.
 * 매개변수
 *  int clnt_sock : 아이디를 전송할 클라이언트 소켓 디스크립터
 * 메시지구조
 *  접속되어 있는 플레이어 아이디 : "id"
 *  더 이상 플레이어가 없음을 알림 : "-0"
 * */
void send_players(int clnt_sock)
{
	printf("[ send_plaeyrs() ] num_players : %d\n", num_players);
	int i;
	for(i = 0; i < num_players; i++)
	{
		my_write(clnt_sock, players[i].id, ID_LEN);
	}
	char end_list[2] = {'-', '0'};
	my_write(clnt_sock, end_list, strlen(end_list));
}



/* 원하는 바이트 수만큼 전송하는 함수.
 * 내부에서 write 함수를 두 번 호출한다. 
 * 첫번째 write는 전송할 바이트 수를 전송.
 * 두번째 write는 메시지 전송.
 * 매개변수
 *  int sock : 데이터 전송할 소켓 디스크립터
 *  void * buf : 전송할 데이터가 담긴 버퍼
 *  int signedLen : 전송할 바이트 수. 채팅 메시지일 경우 양수, 좌표일 경우 음수.
 * 반환값
 *  전송한 바이트 수.(두 번째 write의 반환값) 
 * */
int my_write(int sock, void* buf, int signedLen){
	int value;

	write(sock, &signedLen, sizeof(int));

	//signedLen이 음수일 경우 buf의 내용은 좌표라는 의미이다
	//양수로 바꿔서 write함수의 세 번째 인자로 넣어준다
	if(signedLen < 0){
		signedLen = -signedLen;
		value = write(sock, buf, signedLen);
		return -value;
	}
	value = write(sock, buf, signedLen);
	return value;
}


/* 전송된 바이트 수 만큼 수신하는 함수.
 * 내부에서 read를 두 번 호출한다.
 * 첫번째 read는 메시지 길이를 수신.
 * 두번째 read는 메시지를 수신. 
 * 매개변수
 *  int fd : 메시지 읽어 올 소켓 디스크립터
 *  void * buf : 수신한 메시지를 저장할 버퍼
 *  int buf_size : 버퍼 최대 사이즈
 * 반환값
 *  읽어들인 바이트 수
 * */
int my_read(int fd, void * buf, int buf_size){

	int origin_len = 0;
	int tmp_len;
	int res = 0;

	res = read(fd, &origin_len, sizeof(origin_len));
	//printf("!!!!!!!!!!!!! read1 : %d\n\n", res);
	tmp_len = origin_len;
	if(tmp_len < 0)
		tmp_len = -tmp_len;

	while( tmp_len != 0 && (res = read(fd, buf, tmp_len)) )
	{
		if(res == -1)
		{
			//if (errno == EINTR)
			//	continue;
			//     error_handling("read error");
		}
		tmp_len -= res;
		buf += res;
	}
	return origin_len;
}


/* SIGCHLD 시그널에 대한 시그널 핸들링 함수이다. 해당 프로세스 아이디를 출력하고 프로세스를 종료시킨다. */
void sigchld_handling(int sig)
{
	int status;
	pid_t id = waitpid(-1, &status, WNOHANG);
	if(WIFEXITED(status))
	{
		printf("Removed proc id : %d\n", id);
	}
	sprintf(tmp_log, "Removed proc id : %d", id);
	write_log(-2, getpid(), tmp_log);
}

/* SIGINT 시그널에 대한 시그널 핸들링 함수이다. ‘players’배열에 저장된 소켓 디스크립터들을 종료하고 프로그램을 종료시킨다. */
void sigint_handling(int sig)
{
	printf("[ sigint_handling() ]\n");
	int i;
	for(i = 0; i< num_players; i++)
		close(players[i].fd);
	write_log(-2, getpid(), "sigint handling");
	exit(1);
}

/*
* 기능 : 예외상황을 처리하는 함수이다. 에러 메시지를 출력하고 프로세스를 종료시킨다.
* 매개변수
*	char * msg : 출력할 에러 메시지
*/
void error_handling(char * msg)
{
	printf("%s\n", msg);
	char * tmp_log;
	sprintf("error_handling : %s", msg);
	write_log(-1, getpid(), tmp_log);
	exit(1);
}



/* 파일에 로그를 저장하는 함수.
 * 매개변수
 *  int num : 프로세스 구분하는 번호.
 *  		  0 : 첫번째 서버(accept, keyword 담당) 
 *  		  1 : 두번쩨 서버 (read/write 담당) 
 *  		  2 : 클라이언트 읽기 프로세스(커넥티드 소켓)
 *  		  3 : 클라이언트 쓰기 프로세스(커넥티드 소켓)
 *  		  -1: 에러
 *  		  -2: 시그널핸들링
 *  int pid : 해당 프로세스 아이디
 *  char * msg : 설명
 * 메시지구조
 *  "num / pid / msg"
 * */
void write_log(int num, int pid, char * msg)
{
	char log[70];
	memset(log, 0, 50);
	sprintf(log, "%d / %d / %s\n", num, pid, msg);
	printf("[ write_log() ] : %s\n", log);
	write(log_file, log, strlen(log));	
}

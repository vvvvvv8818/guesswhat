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
	int fd;				//���� ��ũ����
	int score;			//����
	char id[ID_LEN];	//���̵�
	int pipes[2];		//������
}typedef player;
player players[4];	//������ �÷��̾� ����Ʈ
int num_players;	//������ �÷��̾� ��

int fdc[2];	//��� ���μ��� -> read server
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
	write(log_file, "��ȣ/pid/����\n", 14);
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

	/* 1�� fork() : ����, ä�� ���μ��� �и� */
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
				printf("read server return : 1st read == 0\n");//��������
				return 0;
			}
			buf[strlen(buf)] = '\0';

			printf("1) read server : %s\n", buf);

			if( strcmp(buf, "^") == 0 )
			{		//timeout. keyword ��� ������ �˸���
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

			/* ID �Է¹ޱ� ^_T */
			while(1)
			{
				int str_len;
				memset(&buf, 0, BUF_SIZE);
				str_len = my_read(clnt_sock, buf, BUF_SIZE);
				if(str_len == -1)
					error_handling("id error");////��??������ȵ�...
				if( id_exist(buf, str_len) == -1 )//�ߺ��˻�
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
					//�α��� ���� �޽��� ������
					printf("login is failed\n");
					int res = -1;
					my_write(clnt_sock, &res, sizeof(res));
					write_log(0, getpid(), "Login failed.");
					continue;
				}
			}//ID�Է¹ޱ� ��

			/* 2�� fork() : connected process */
			int pid = fork();
			if(pid == 0)//connected
			{
				//printf("3) 3��° fork()\n");
				write_log(0, getpid(), "This is connected process.");
				close(serv_sock);
				/* 3�� fork() : ����� ���μ��� �и� */
				pid = fork();
				if(pid == 0)//read���
				{
					write_log(2, getpid(), "This is connected read process.");
					connected_read(clnt_sock);
					break;
				}
				else//write���
				{
					write_log(3, getpid(), "This is connected write process.");
					connected_write(clnt_sock);
					break;
				}
				printf("3�� fork() ���� return(2�� fork()�� �ڽ� ���μ���)\n");
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


/* ������ �����Ѵ�. GAME_NUM��ŭ ���� �ϸ鼭
 * Ű����(����)�� �������� ���� ������(drawer)�� ���̵�� �Բ� �����ϰ�
 * �÷��̾��κ��� ���� �޽����� ����� ���Ͽ� ����ó���� �Ѵ�.
 * �Ű�����
 *  char * buf : ����
 * �޽�������
 *  ���ο� ���� ���۵��� �˸� : "*"
 *  Ŭ���̾�Ʈ�κ��� ���� ä�� �޽��� : "[id] msg"
 *  ������ ���� : "<drawer_id> keyword"
 *  ���� �˸� : "<player_id> correct!" 
 * */
void start_game(char * buf)
{
	printf("[ start_game() ]\n");
	int current_keyword;
	int game_cnt;
	int drawer;		//current drawer index
	int is_selected[KEYWORD_NUM];	//���� �ߺ����� ���� �ʵ��� Ȯ��
	memset(is_selected, 0, sizeof(is_selected));

	//GUME_NUM��ŭ ���� ����(����)
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
		//pipe���� �о Ȯ��
		while(1)
		{
			memset(buf, 0, BUF_SIZE);
			int res = my_read(fds[0], buf, BUF_SIZE);
			write_log(0, getpid(), buf);
			printf("### res : %d\n", res);
			if( res == 0)
			{
				continue;//EOF //��� ����
			}
			else if( res == -1 )
				error_handling("keyword checking error\n");
			if( !strcmp(buf, "^") )
			{
				printf("# timeout");
				write_log(0, getpid(), "timeout");
				break; //while loop ��������
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
						break;	//������ ����߸� while�� ��������
					}
				}
			}//if-else end
		}//while end
		printf("~ to next keyword..\n");
	}//for end
	char * end_msg = "~ ���� �� ~";
	my_write(fdc[1], end_msg, strlen(end_msg));
	printf("end of game_start()\n");
}



/* Ű����(����)�� �������� �����ؼ� �������� ������ �Լ�
 * �Ű�����
 *  int is_selected[] : ���õǾ��� Ű���� �ε����� �����ϴ� �迭
 *  int drawer : ���� �������� players �迭������ �ε���
 * ���ϰ�
 *  ���õ� Ű������ �ε���
 * �޽��� ���� : "<id of drawer> keyword" 
 * */
int send_keyword(int is_selected[], int drawer)
{
	printf("[ send_keyword() ]\n");
	/* ���� ���� : "<drawer id> keyword" */
	//read server�� write
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
	//?ڿ??���� ������?�ٿ��� write 
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


/* Ŭ���̾�Ʈ�κ��� �� �����͸� ID�� �޽����� �����Ͽ� ����
 * �Ű�����
 *  char buf[] : Ŭ���̾�Ʈ�κ��� �� ������
 *  char * id : ID�� ������ ������
 *  char * msg : �޽����� ������ ������
 * ���ϰ�
 *  ������ 1, Ŭ���̾�Ʈ�κ��� �� �޽����� �ƴ� ��� -1 ����.
 * �޽�������
 *  Ŭ���̾�Ʈ�κ��� �� ������ : "[id] msg"
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


/* ���� ���� ������ ���� socket(), bind() ������ ó���ϴ� �Լ� 
 * �Ű�����
 *  int * serv_sock : ���� ���� ��ũ���� ������ ����
 *  struct sockaddr_in * serv_addr : ���� ���� �ּ����� ������ ����
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


/* Ŭ���̾�Ʈ�� ����� Ŀ��Ƽ�� ������ ����ϴ� ���μ������� ȣ���ϴ� �Լ�.
 * Ŭ���̾�Ʈ�κ��� �޽����� �о�� �������� �����Ѵ�.
 * �Ű�����
 *  int clnt_sock : Ŭ���̾�Ʈ ���� ��ũ����
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

/* Ŭ���̾�Ʈ�� ����� Ŀ��Ƽ�� ������ ����ϴ� ���μ������� ȣ���ϴ� �Լ�.
 * ����(������)�κ��� �޽����� �о� Ŭ���̾�Ʈ���� �����Ѵ�.
 * �Ű�����
 *  int clnt_sock : Ŭ���̾�Ʈ ���� ��ũ����
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


/* �Ű������� ���� ���̵� �ߺ��Ǵ��� Ȯ�� 
 * �Ű�����
 *  char * input : ���̵�
 *  int len : input�� ����
 * ���ϰ�
 *  �ߺ��Ǹ� �ߺ��Ǵ� �÷��̾��� �ε���, �ߺ����� ������ -1�� ����
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



/* �÷��̾� �迭(players)�� ���ο� �÷��̾ �߰��ϴ� �Լ� 
 * num_players���� 1 ������ 
 * �Ű�����
 *  int clnt_sock : ���ο� Ŭ���̾�Ʈ ���� ��ũ����
 *  char * buf : ���ο� Ŭ���̾�Ʈ ���̵� ��� ����
 *  int str_len : ���� ����
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


/* ���ο� �÷��̾�(���� �ֱٿ� ����� �÷��̾�)�� id�� read server���� ����
 * read server���� ���ο� �÷��̾ ���������� �players �迭�� ��� �ÿ��̾�� �˸��� 
 * �޽�������
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


/* ���ο� �÷��̾�� ���ӵǾ� �ִ� �÷��̾��� ���̵� ����.
 * �Ű�����
 *  int clnt_sock : ���̵� ������ Ŭ���̾�Ʈ ���� ��ũ����
 * �޽�������
 *  ���ӵǾ� �ִ� �÷��̾� ���̵� : "id"
 *  �� �̻� �÷��̾ ������ �˸� : "-0"
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



/* ���ϴ� ����Ʈ ����ŭ �����ϴ� �Լ�.
 * ���ο��� write �Լ��� �� �� ȣ���Ѵ�. 
 * ù��° write�� ������ ����Ʈ ���� ����.
 * �ι�° write�� �޽��� ����.
 * �Ű�����
 *  int sock : ������ ������ ���� ��ũ����
 *  void * buf : ������ �����Ͱ� ��� ����
 *  int signedLen : ������ ����Ʈ ��. ä�� �޽����� ��� ���, ��ǥ�� ��� ����.
 * ��ȯ��
 *  ������ ����Ʈ ��.(�� ��° write�� ��ȯ��) 
 * */
int my_write(int sock, void* buf, int signedLen){
	int value;

	write(sock, &signedLen, sizeof(int));

	//signedLen�� ������ ��� buf�� ������ ��ǥ��� �ǹ��̴�
	//����� �ٲ㼭 write�Լ��� �� ��° ���ڷ� �־��ش�
	if(signedLen < 0){
		signedLen = -signedLen;
		value = write(sock, buf, signedLen);
		return -value;
	}
	value = write(sock, buf, signedLen);
	return value;
}


/* ���۵� ����Ʈ �� ��ŭ �����ϴ� �Լ�.
 * ���ο��� read�� �� �� ȣ���Ѵ�.
 * ù��° read�� �޽��� ���̸� ����.
 * �ι�° read�� �޽����� ����. 
 * �Ű�����
 *  int fd : �޽��� �о� �� ���� ��ũ����
 *  void * buf : ������ �޽����� ������ ����
 *  int buf_size : ���� �ִ� ������
 * ��ȯ��
 *  �о���� ����Ʈ ��
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


/* SIGCHLD �ñ׳ο� ���� �ñ׳� �ڵ鸵 �Լ��̴�. �ش� ���μ��� ���̵� ����ϰ� ���μ����� �����Ų��. */
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

/* SIGINT �ñ׳ο� ���� �ñ׳� �ڵ鸵 �Լ��̴�. ��players���迭�� ����� ���� ��ũ���͵��� �����ϰ� ���α׷��� �����Ų��. */
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
* ��� : ���ܻ�Ȳ�� ó���ϴ� �Լ��̴�. ���� �޽����� ����ϰ� ���μ����� �����Ų��.
* �Ű�����
*	char * msg : ����� ���� �޽���
*/
void error_handling(char * msg)
{
	printf("%s\n", msg);
	char * tmp_log;
	sprintf("error_handling : %s", msg);
	write_log(-1, getpid(), tmp_log);
	exit(1);
}



/* ���Ͽ� �α׸� �����ϴ� �Լ�.
 * �Ű�����
 *  int num : ���μ��� �����ϴ� ��ȣ.
 *  		  0 : ù��° ����(accept, keyword ���) 
 *  		  1 : �ι��� ���� (read/write ���) 
 *  		  2 : Ŭ���̾�Ʈ �б� ���μ���(Ŀ��Ƽ�� ����)
 *  		  3 : Ŭ���̾�Ʈ ���� ���μ���(Ŀ��Ƽ�� ����)
 *  		  -1: ����
 *  		  -2: �ñ׳��ڵ鸵
 *  int pid : �ش� ���μ��� ���̵�
 *  char * msg : ����
 * �޽�������
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

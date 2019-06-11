#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <time.h>
#pragma warning(disable:4996)

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define BUF_SIZE 100
#define MAX_CLNT 3
#define ElapsedTime 20

typedef struct {
	int x; // 단어의 x좌표
	char word[20]; // 단어 저장
} rain;

typedef struct {
	int is_used; //1: 사용된 단어, 0: 사용되지 않은 단어
	char word[30];
}words;

unsigned WINAPI start_game(void* arg);
void acidRain(void);
void initRain(void);
void error_handling(char* msg);
void menu(char port[]);

int clnt_cnt = 0;
int start_flag = 0;
char adrs[100] = "123456";
SOCKET clnt_socks[MAX_CLNT];
HANDLE mutx;
HANDLE cannot_mutx;

double sec = 0.0;
double ph[3] = { 7.0, };
int wait_for_sending_result = 0;
char msg[MAX_CLNT][BUF_SIZE] = { 0 };
int msg_index = 0;

rain rains[21];
words word_info[200];

int main()
{
	WSADATA wsaData;
	SOCKET serv_sock, clnt_sock;
	SOCKADDR_IN serv_adr, clnt_adr;
	int clnt_adr_sz;
	int i = 0;
	HANDLE t_id[MAX_CLNT];
	FILE* word_data;

	//initialize word_info array
	word_data = fopen("text_data.txt", "r");
	while (feof(word_data) == 0) {
		fscanf(word_data, "%s", word_info[i].word);
		word_info[i].is_used = 0;
		i++;
	}
	fclose(word_data);

	/** time log **/
	struct tm* t;
	time_t timer = time(NULL);
	t = localtime(&timer);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		error_handling("WSAStartup() error!");

	mutx = CreateMutex(NULL, FALSE, NULL);
	cannot_mutx = CreateMutex(NULL, FALSE, NULL);

	menu(adrs);

	//pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(adrs));

	if (bind(serv_sock, (SOCKADDR*)& serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		error_handling("bind() error");

	if (listen(serv_sock, 5) == SOCKET_ERROR)
		error_handling("listen() error");

	while (1) {
		if (clnt_cnt < MAX_CLNT) {
			t = localtime(&timer);
			clnt_adr_sz = sizeof(clnt_adr);
			clnt_sock = accept(serv_sock, (SOCKADDR*)& clnt_adr, &clnt_adr_sz);

			WaitForSingleObject(mutx, INFINITE);
			clnt_socks[clnt_cnt++] = clnt_sock;
			ReleaseMutex(mutx);

			t_id[clnt_cnt - 1] = (HANDLE)_beginthreadex(NULL, 0, start_game, (void*)& clnt_sock, 0, NULL);

			printf(" Connceted client IP : %s ", inet_ntoa(clnt_adr.sin_addr));
			printf("(%d-%d-%d %d:%d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
				t->tm_hour, t->tm_min);
			printf(" player (%d/%d)\n", clnt_cnt, MAX_CLNT);
		}

		//Sleep(100);
		if (clnt_cnt == MAX_CLNT) {
			start_flag = 1;
			break;
		}
	}
	
	WaitForMultipleObject(t_id, INFINITE);


	closesocket(serv_sock);
	return 0;
}

unsigned WINAPI start_game(void* arg)
{
	SOCKET sock = *((SOCKET*)arg);
	HANDLE thr_id;
	double start, end; // 게임 시작 시간, 단어가 생성된 시간 기록
	double sec; // 진행시간
	rain end_game = { -1,"%end%" };
	int i, out = 0;
	char score_info[100], player_name[21];

	i = recv(sock, player_name, sizeof(player_name), 0);
	player_name[i] = '\0';
	printf("[notice]'%s' is ready\n", player_name);
	
	//wait for three players
	//만약 게임 대기 중에 클라이언트가 나간다면, 소켓을 종료시키고 새로운 클라이언트를 받아야한다.
	while (start_flag == 0) {
		Sleep(1000);
			if (send(sock, "-", sizeof("-"), 0) == SOCKET_ERROR) {
				WaitForSingleObject(mutx, INFINITE);
				clnt_cnt--;
				ReleaseMutex(mutx);
				printf("[notice]'%s' is out\n\n", player_name);
				closesocket(sock);
				printf("out\n");
				return NULL;
			}
	}

	if (send(sock, "start_g", strlen("start_g"), 0) == SOCKET_ERROR) {
		out = 1;
	}

	initRain();

	start = clock();

	while (out != 1) {
		Sleep(1000);

		WaitForSingleObject(mutx, INFINITE);
		acidRain();
		ReleaseMutex(mutx);
		end = clock();
		sec = (double)(end - start) / CLOCKS_PER_SEC;

		
		if (sec >= ElapsedTime) {
			break;
		}

		// client에게 구조체 넘기기
		if (send(sock, (char*)& rains, sizeof(rain), 0) == SOCKET_ERROR) {
			//클라이언트가 단어를 뿌려주는 와중에 떠났다면,
			out = 1;
			break;
		}
	}

	sprintf(score_info, "player<<%s>>\tleft the game", player_name);

	//단어 뿌리기가 끝난 후 클라이언트가 종료했다면,
	if (out != 1)
		//alert game is over to client
		if (send(sock, (char*)& end_game, sizeof(rain), 0) == SOCKET_ERROR) {
			out = 1;
		}

	//단어 뿌리기가 끝난 후 클라이언트가 종료했다면,
	if (out != 1) {
		//receive client's score
		//score_info[0] = '\0';
		i = recv(sock, score_info, sizeof(score_info), 0);
		if (i == SOCKET_ERROR) {
			out = 1;
		}
		else
			score_info[i] = '\0';
	}

	if (out == 1)
		printf("[notice]: player <<%s>> left the game\n", player_name);

	//msg에 각 플레이어의 점수 정보를 입력한다.
	for (i = 0; i < clnt_cnt; i++) {
		if (sock == clnt_socks[i]) {
			WaitForSingleObject(mutx, INFINITE);
			if (out != 1)
				wait_for_sending_result++;
			strcpy(msg[msg_index++], score_info);
			ReleaseMutex(mutx);
			break;
		}
	}

	//도중에 떠나지 않은 플레이어의 점수를 모두 받기 위해 기다린다.
	while (1) {
		if (wait_for_sending_result == clnt_cnt || out == 1)
			break;
	}

	//도중에 떠나지 않은 플레이어들에게 점수를 전달한다.
	if (out != 1) {
		WaitForSingleObject(mutx, INFINITE);
		for (i = 0; i < MAX_CLNT; i++) {
			if (send(sock, msg[i], strlen(msg[i]), 0) == SOCKET_ERROR)
				error_handling("sending error");

		}
		ReleaseMutex(mutx);
	}
	// remove disconnected client
	WaitForSingleObject(mutx, INFINITE);
	for (i = 0; i < clnt_cnt; i++) {
		if (sock == clnt_socks[i]) {

			while (i < clnt_cnt - 1) {
				clnt_socks[i] = clnt_socks[i + 1];
				i++;
			}
			clnt_cnt--;
			if (out != 1)
				wait_for_sending_result--;
			break;
		}
	}
	ReleaseMutex(mutx);
	/* ph 농도(점수)가 제일 높은 사람이 진다
	 단어를 맞혔으면 점수 유지
	 단어가 선을 넘어가면 점수 깎기(산성화)*/

	closesocket(sock);

	return NULL;
}

void acidRain()
{
	int i;
	int rand_num;

	srand(time(NULL));

	for (i = 20; i > 0; i--) {
		strcpy(rains[i].word, rains[i - 1].word);
		rains[i].x = rains[i - 1].x;
		rains[i - 1].x = 0;
		strcpy(rains[i - 1].word, " ");
	}


	WaitForSingleObject(mutx,INFINITE);
	rains[0].x = (rand() % 8) * 11 + (rand() % 5) * 9;
	srand(time(NULL));
	rand_num = rand() % 200;
	ReleaseMutex(mutx);
	while (word_info[rand_num].is_used == 1)
		rand_num = rand() % 200;
	word_info[rand_num].is_used = 1;

	strcpy(rains[0].word, word_info[rand_num].word);

}

void initRain()
{
	int i;

	for (i = 0; i < 21; i++) {
		rains[i].x = 0;
		strcpy(rains[i].word, " ");
	}
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void menu(char port[])
{
	system("cls");
	printf(" **** acid rain server ****\n");
	printf(" server port    : %s\n", port);
	printf(" max connection : %d\n", MAX_CLNT);
	printf(" ****          Log         ****\n\n");
}
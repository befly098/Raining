#pragma comment(lib, "ws2_32.lib");
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <time.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define BUF_SIZE 100
#define MAX_CLNT 3
#define ElapsedTime 90

typedef struct {
	int x; // 단어의 x좌표
	char word[20]; // 단어 저장
} rain;

unsigned WINAPI start_game(void *arg);
void acidRain(void);
void initRain(void);
unsigned WINAPI remove(void *data);
// void send_msg(char *msg, int len);
void error_handling(char *msg);
void menu(char port[]);

int clnt_cnt = 0;
int start_flag = 0;
char adrs[100] = "123456";
SOCKET clnt_socks[MAX_CLNT];
HANDLE mutx;
HANDLE cannot_mutx;

double sec = 0.0;
double ph[3] = { 7.0, };
int threadExit = 1;
char msg[MAX_CLNT][BUF_SIZE] = { 0, };

rain rains[21];
char *words[35] = { // 단어 표본은 총 35개국의 수도 이름
   "network",
   "windows.h",
   "thread",
   "WSASocket",
   "server",
   "client",
   "bind",
   "connect",
   "closesocket",
   "TCP",
   "ws2_32.lib",
   "TTL",
   "MULTICAST",
   "SO_REUSEADDR",
   "getsockopt",
   "select",
   "Overlapped IO",
   "Nagle",
   "IOCP",
   "blocking",
   "asynchronous",
   "notification",
   "signaled",
   "FD_ACCEPT",
   "WSACreateEvent",
   "WSACloseEvent",
   "mutex",
   "semapohre",
   "critical section",
   "port",
   "UDP",
   "error",
   "이정원",
   "우지현",
   "박서린"
};

int main()
{
	WSADATA wsaData;
	SOCKET serv_sock, clnt_sock;
	SOCKADDR_IN serv_adr, clnt_adr;
	int clnt_adr_sz;
	HANDLE t_id, tt_id;

	/** time log **/
	struct tm *t;
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

	if (bind(serv_sock, (SOCKADDR*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		error_handling("bind() error");

	if (listen(serv_sock, 5) == SOCKET_ERROR)
		error_handling("listen() error");

	while (1) {
		if (clnt_cnt < MAX_CLNT) {
			t = localtime(&timer);
			clnt_adr_sz = sizeof(clnt_adr);
			clnt_sock = accept(serv_sock, (SOCKADDR*)&clnt_adr, &clnt_adr_sz);

			WaitForSingleObject(mutx, INFINITE);
			clnt_socks[clnt_cnt++] = clnt_sock;
			ReleaseMutex(mutx);

			t_id = (HANDLE)_beginthreadex(NULL, 0, start_game, (void*)&clnt_sock, 0, NULL);
			WaitForSingleObject(t_id, INFINITE);
			printf(" Connceted client IP : %s ", inet_ntoa(clnt_adr.sin_addr));
			printf("(%d-%d-%d %d:%d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
				t->tm_hour, t->tm_min);
			printf(" chatter (%d/%d)\n", clnt_cnt, MAX_CLNT);
		}
		
		if (clnt_cnt == MAX_CLNT)
			start_flag = 1;
	}

	closesocket(serv_sock);
	return 0;
}

unsigned WINAPI start_game(void *arg)
{
	SOCKET sock = *((SOCKET*)arg);
	HANDLE thr_id;
	double start, end; // 게임 시작 시간, 단어가 생성된 시간 기록
	double sec; // 진행시간
	int num = clnt_cnt;
	int i;

	initRain();
	
	threadExit = 0;
	thr_id = (HANDLE)_beginthreadex(NULL, 0, remove, (void*)&sock, 0, NULL);

	start = clock();

	while (1) {
		sleep(100);

		acidRain();
		end = clock();
		sec = (double)(end - start) / CLOCKS_PER_SEC;
		

		if (sec >= ElapsedTime) {
			threadExit = 1;
			WaitForSingleObject(thr_id, INFINITE);
			break;
		}

		// client에게 구조체 넘기기
		if (send(sock, (char *)&rains, sizeof(rain), 0) == SOCKET_ERROR) {
			error_handling("send() struct error");
			exit(1);
		}
	}

	// remove disconnected client
	/*WaitForSingleObject(mutx, INFINITE);
	for (i = 0; i < clnt_cnt; i++) {
		if (sock == clnt_socks[i]) {
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}*/

	/* ph 농도(점수)가 제일 높은 사람이 진다
	 단어를 맞혔으면 점수 유지
	 단어가 선을 넘어가면 점수 깎기(산성화)*/

	clnt_cnt--;
	ReleaseMutex(mutx);

	closesocket(sock);
	return NULL;
}

void acidRain()
{
	int i;

	srand(time(NULL));

	for (i = 20; i >= 0; i--) {
		strcpy(rains[i].word, rains[i - 1].word);
		rains[i].x = rains[i - 1].x;
		rains[i - 1].x = 0;
		strcpy(rains[i - 1].word, " ");
	}

	rains[0].x = rand() % 80 + 50;
	srand(time(NULL));
	strcpy(rains[0].word, words[rand() % 35]);
}

void initRain()
{
	int i;

	for (i = 0; i < 21; i++) {
		rains[i].x = 0;
		strcpy(rains[i].word, " ");
	}
}

unsigned WINAPI remove(void *data)
{
	int i;

	while (!threadExit) {
		gets(msg);
		for (i = 0; i < 20; i++) {
			if (strstr(rains[i].word, msg)) {
				strcpy(rains[i].word, " ");

			}
		}
	}
}

/*void send_msg(char* msg, int len)
{
	int i;

	WaitForSingleObject(mutx, INFINITE);
	for (i = 0; i < clnt_cnt; i++)
		send(clnt_socks[i], msg, len, 0);
	msg[0] = '\0';
	ReleaseMutex(mutx);
}*/

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void menu(char port[])
{
	system("cls");
	printf(" **** moon/sun chat server ****\n");
	printf(" server port    : %s\n", port);
	printf(" max connection : %d\n", MAX_CLNT);
	printf(" ****          Log         ****\n\n");
}
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
#define _CRT_NONSTDC_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <time.h> 
#include"ConsoleDrawing.h"
#define BUF_SIZE 100
#define NORMAL_SIZE 21
#define LINES 200
#define MAX_CLNT 3

typedef struct { // 산성비(단어) 구조체
	int x; // 단어의 x좌표
	char word[20]; // 단어 저장 공간
} rain;

unsigned WINAPI send_msg(void* arg);
unsigned WINAPI recv_msg(void* arg);
void move(int, int);
void error_handling(char* msg);
void prnscreen();
rain* recv_rain(void* arg);


char name[NORMAL_SIZE];     // name
char address[BUF_SIZE];     // ip address
char serv_time[NORMAL_SIZE];        // server time
char msg[BUF_SIZE] = { 0 };                    // msg
char result_msg[MAX_CLNT][BUF_SIZE] = { 0 };

int line = 21;// 20번째 rains는 판정선.(0~19번째까지만 화면 표시)
rain rains[21];
double sec = 0.0; // 플레이 누적 시간
double ph = 7.0; // 수소 이온 농도
int flag = 0;
int end_flag = 0, arr_empty = 0;

HANDLE mutx;

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN serv_addr;
	HANDLE snd_thread, rcv_thread;

	printf("이름을 입력하세요 : ");
	scanf("%s", name);
	printf("IP 주소를 입력하세요 : ");
	scanf("%s", address);

	/** local time **/
	struct tm* t;
	time_t timer = time(NULL);
	t = localtime(&timer);
	sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour,
		t->tm_min);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		error_handling("WSAStartup() error!");

	sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(address);
	serv_addr.sin_port = htons(atoi("123456"));

	if (connect(sock, (SOCKADDR*)& serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		error_handling("conncet() error");


	system("cls");
	printf("waiting for another player...\n");
	mutx = CreateMutex(NULL, FALSE, NULL);
	send(sock, name, strlen(name), 0);

	rcv_thread = (HANDLE)_beginthreadex(NULL, 0, recv_msg, (void*)& sock, 0, NULL);
	snd_thread = (HANDLE)_beginthreadex(NULL, 0, send_msg, (void*)& sock, 0, NULL);
	WaitForSingleObject(snd_thread, INFINITE);
	WaitForSingleObject(rcv_thread, INFINITE);



	closesocket(sock);
	//	endwin();
	return 0;
}

unsigned WINAPI send_msg(void* arg)
{
	int sock = *((int*)arg);
	int i, j;
	char c;
	char* who = NULL;
	char temp[BUF_SIZE];

	system("mode con:cols=200 lines=100 | title 산성비");
	printf("waiting for another player...\n");

	while (1)
	{
		scanf("%s", msg);

		if ((end_flag == 1) && (arr_empty == -1)) {
			break;
		}

		move(1, 25);
		printf("                                                                                                                                                                                            ");
		move(1, 25);

		msg[strlen(msg)] = '\0';

		for (i = 0; i < 21; i++) {
			if (!strcmp(rains[i].word, msg))
				strcpy(rains[i].word, " ");
		}

		msg[0] = '\0';
	}
	system("cls");
	printf("loading the result...");
	msg[0] = '\0';
	sprintf(msg, "player<<%s>>:\t%.1lf", name, ph);
	msg[strlen(msg)] = '\0';
	//send result to server
	send(sock, msg, strlen(msg), 0);
	//get all result from server
	for (i = 0; i < MAX_CLNT; i++) {
		j = recv(sock, temp, sizeof(temp), 0);
		temp[j] = '\0';
		if (i == 0) {
			system("cls");
			printf("------------------------<<result>>------------------------\n\n");
		}
		Sleep(100);
		printf("\n\n%s", temp);
	}

	return NULL;
}

unsigned WINAPI recv_msg(void* arg)
{
	int i;
	int sock = *((int*)arg);
	int str_len;

	for (i = 0; i < 21; i++)
		strcpy(rains[i].word, " ");
	while (1) {

		//check if all word is disappeared
		if (end_flag == 1) {
			Sleep(1000);
			for (i = 0; i < 21; i++) {
				if (strcmp(rains[i].word, " ") != 0) {
					arr_empty = 1;
					break;
				}
			}

			if (arr_empty == 0) {
				arr_empty = -1;
				break;
			}
			arr_empty = 0;
		}

		for (i = 20; i > 0; i--)
		{
			strcpy(rains[i].word, rains[i - 1].word); // 기존 단어는 한 줄씩 밀고
			rains[i].x = rains[i - 1].x;
			rains[i - 1].x = 0;
			memset(rains[i - 1].word, '0', 20); // 뒷 줄은 공백처리
			strcpy(rains[i - 1].word, " ");
		}

		if (end_flag == 0) {
			rains[0] = recv_rain((void*)& sock)[0];
			rains[0].word[strlen(rains[0].word)] = '\0';
		}

		if ((rains[0].x == -1) && (end_flag == 0)) {
			end_flag = 1;
			strcpy(rains[0].word, " ");
			rains[0].x = 0;
		}
		prnscreen();

	}

	system("cls");
	printf("Insert any alphabet to see result....>");

	return NULL;
}

void move(int x, int y)
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(consoleHandle, pos);
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

rain* recv_rain(void* arg)
{
	int sock = *((int*)arg);
	int retval;
	char suBuffer[100];

	retval = recv(sock, (char*)& suBuffer, sizeof(rain), 0);
	if (retval == SOCKET_ERROR)
	{
		error_handling("recv() error!");
	}

	rain* rains_temp;

	suBuffer[retval] = '\0';
	rains_temp = (rain*)suBuffer;

	return rains_temp;
}

void prnscreen() // 화면 출력 함수
{
	int i;
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int x, y;
	if (flag == 1) {
		x = coord_x();
		y = coord_y();
		//system("cls");
	}
	move(0, 0);
	for (i = 0; i < 23; i++) {
		move(0, i);
		printf("                                                                                                                                                                                            ");
	}
	move(0, 0);
	printf("player: <<%s>>", name);
	move(0, 1);
	printf("==================================================================================================================================================");
	for (i = 0; i < 20; i++)
	{
		move(0, 1 + (i + 1));
		printf("%*s", rains[i].x, rains[i].word); // x좌표에 맞추어 가변적으로 단어 출력
	}
	move(0, 22);
	printf("~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~"); // 판정선(rains[20].word)
	if (strcmp(rains[20].word, " ")) // 판정선에 단어가 남아있으면
		ph -= 0.5; // 0.5씩 산성화 시킴

	move(0, 23);
	printf("[ pH ] %.1lf  [ 시간 ] %.2lf초", ph, sec++); // 수소 이온 농도와 총 시간 출력

	move(0, 25);
	printf(">");

	if (flag == 1) {
		move(x, y);
	}

	flag = 1;

}
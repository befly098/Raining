#pragma comment(lib, "ws2_32.lib");
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <time.h> 
#define BUF_SIZE 100
#define NORMAL_SIZE 20
#define LINES 200

unsigned WINAPI send_msg(void* arg);
unsigned WINAPI recv_msg(void* arg);
void move(int, int);
void error_handling(char* msg);


char name[NORMAL_SIZE];     // name
char msg_form[NORMAL_SIZE];            // msg form
char serv_time[NORMAL_SIZE];        // server time
char msg[BUF_SIZE];                    // msg

typedef struct { // �꼺��(�ܾ�) ����ü
	int x; // �ܾ��� x��ǥ
	char word[20]; // �ܾ� ���� ����
} rain;

int line = 21;// 20��° rains�� ������.(0~19��°������ ȭ�� ǥ��)
rain *rains;
double sec = 0.0; // �÷��� ���� �ð�
double ph = 7.0; // ���� �̿� ��

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN serv_addr;
	HANDLE snd_thread, rcv_thread;

	printf("�̸��� �Է��ϼ��� : ");
	scanf("%s", name);

	/** local time **/
	struct tm *t;
	time_t timer = time(NULL);
	t = localtime(&timer);
	sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour,
		t->tm_min);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		error_Handling("WSAStartup() error!");

	sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(atoi("123456"));

	if (connect(sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		error_handling("conncet() error");


	system("cls");

	snd_thread = (HANDLE)_beginthreadex(NULL, 0, send_msg, (void*)&sock, 0, NULL);
	rcv_thread = (HANDLE)_beginthreadex(NULL, 0, recv_msg, (void*)&sock, 0, NULL);
	WaitForSingleObject(snd_thread, INFINITE);
	WaitForSingleObject(rcv_thread, INFINITE);



	closesocket(sock);
	endwin();
	return 0;
}

unsigned WINAPI send_msg(void* arg)
{
	SOCKET sock = *((SOCKET*)arg);
	char name_msg[NORMAL_SIZE + BUF_SIZE];
	char myInfo[BUF_SIZE];
	char* who = NULL;
	char temp[BUF_SIZE];
	/** send join messge **/
	//sprintf(myInfo, "%s's join. IP_%s\n",name , clnt_ip);
	send(sock, myInfo, strlen(myInfo), 0);

	//system("Acid Raining");
	system("mode con:cols=200 lines=200 | title �꼺��");

	while (1) {
		move(LINES - 50, 0);
		printf("--------------------------------------------------------------------");
		move(LINES - 49, 0);
		printf("q or Q to quit: ");
		scanf("%s", msg);
		printf(msg);
		//fgets(msg, BUF_SIZE, stdin);

		if (!strcmp(msg, "q") || !strcmp(msg, "Q")) {
			closesocket(sock);
			exit(0);
		}

		// send message
		//sprintf(name_msg, "%s %s", name,msg);
		send(sock, msg, strlen(msg), 0);
		msg[0] = '\0';
	}
	return NULL;
}

unsigned WINAPI recv_msg(void* arg)
{
	int sock = *((int*)arg);
	char name_msg[NORMAL_SIZE + BUF_SIZE];
	int str_len;

	while (1) {
		str_len = recv(sock, name_msg, NORMAL_SIZE + BUF_SIZE - 1, 0);
		if (str_len == -1)
			return (void*)-1;
	}
	return NULL;
}

void move(int x, int y)
{
	HANDLE consoleHandle = GetStdHAndle(STD_OUTPUT_HANDLE);
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

rain recv_rain(SOCKET sock)
{
	int retval;
	char suBuffer[21];

	retval = recv(sock, suBuffer, line, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv() error!");
	}

	rain *rains_temp;

	suBuffer[retval] = '\0';
	rains_temp = (rain*)suBuffer;

	return *rains_temp;
}

void prnscreen() // ȭ�� ��� �Լ�
{
	int i;
	system("cls"); // �ܼ�â �ʱ�ȭ
	for (i = 0; i < 20; i++)
	{
		printf("%*s%s\n", rains[i].x, "", rains[i].word); // x��ǥ�� ���߾� ���������� �ܾ� ���
	}
	printf("~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~"); // ������(rains[20].word)
	if (strcmp(rains[20].word, " ")) // �������� �ܾ ����������
		ph -= 0.5; // 0.5�� �꼺ȭ ��Ŵ
	printf("\n[ pH ] %.1lf  [ �ð� ] %.2lf��\n\n", ph, sec); // ���� �̿� �󵵿� �� �ð� ���
	printf("�Է�>"); // ������� �Էº�
}
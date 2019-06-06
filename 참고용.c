#pragma warning(disable: 4996) // scanf, gets, fopen���� �Լ� ���� ����� ��� ����
#include <stdio.h> // �⺻ ��� ����
#include <stdlib.h> // �ý��� �Լ� ȣ�� ��
#include <conio.h> // gets�Լ� ȣ�� ��
#include <string.h> // ���ڿ� �Լ� ȣ�� ��
#include <time.h> // time�Լ��� clock�Լ� ȣ�� ��
#include <windows.h> // PlaySound �Լ� ȣ�� ��
#include <mmsystem.h> // PlaySound �Լ� ȣ��
#pragma comment(lib, "winmm.lib") // PlaySound �Լ� ȣ�� ��
#define HAVE_STRUCT_TIMESPEC // windows.h�� pthread.h���� �ð� ����ü ������ ���� ������
#include <pthread.h>

#define BGMPATH "AcidRain.mp3" // ��������� ���(���� �̸�)
#define SPEED 1000 // �ܾ �����Ǵ� ���� (���� : ms). �����Ͽ� ���̵� ���� ����. 500���ϴ� �÷��� �����.

char *words[35] = { // �ܾ� ǥ���� �� 35������ ���� �̸�
	"����",
	"�ϰ�",
	"����",
	"����췽",
	"��ī��",
	"����",
	"����",
	"�ĸ�",
	"�Ͻ��׸���",
	"���",
	"����θ�ũ",
	"������",
	"��",
	"�ٸ�����",
	"������",
	"���Ƽ�����",
	"�δ��佺Ʈ",
	"������",
	"����ļ��ũ",
	"�����ϰ�",
	"������",
	"����Ȧ��",
	"���Ű",
	"Ż��",
	"����",
	"������",
	"���帮��",
	"�θ�",
	"���׳�",
	"��Ÿ��",
	"������",
	"�߽��ڽ�Ƽ",
	"��Ƽ�ư�",
	"ĵ����",
	"������"
};

typedef struct { // �꼺��(�ܾ�) ����ü
	int x; // �ܾ��� x��ǥ
	char word[20]; // �ܾ� ���� ����
} rain;

rain rains[21]; // 20��° rains�� ������.(0~19��°������ ȭ�� ǥ��)
clock_t start, end; // ���� ����, ����(����) �ð�
double sec = 0.0; // �÷��� ���� �ð�
double ph = 7.0; // ���� �̿� ��
char buffer[50]; // ����� �Է� ����

void help(); // ���� �Լ�
void viewlog(); // ���� ���� �Լ�
void initrains(); // �ܾ� �迭 �ʱ�ȭ �Լ�
void gamemain(); // ���� �� �Լ�
void prnscreen(); // ȭ�� ��� �Լ�
void makerain(); // �� �ܾ� ���� �Լ�

static pthread_t p_thread; // ������ �̸�
static int thr_id; // �������� ���̵�
static int thr_exit = 1; // ������ ���� ���� ����

void *t_function(void *data); // ������ȭ �� ����� �Է� �Լ�
void start_thread(); // ������ ���� �Լ�
void end_thread(); // ������ ���� �Լ�

int main(void)
{
	int choice = 0;


	printf("\n\n\n"); // ù ȭ��
	printf("        ###     ######  #### ########     ########     ###    #### ##    ## \n");
	printf("       ## ##   ##    ##  ##  ##     ##    ##     ##   ## ##    ##  ###   ## \n");
	printf("      ##   ##  ##        ##  ##     ##    ##     ##  ##   ##   ##  ####  ## \n");
	printf("     ##     ## ##        ##  ##     ##    ########  ##     ##  ##  ## ## ## \n");
	printf("     ######### ##        ##  ##     ##    ##   ##   #########  ##  ##  #### \n");
	printf("     ##     ## ##    ##  ##  ##     ##    ##    ##  ##     ##  ##  ##   ### \n");
	printf("     ##     ##  ######  #### ########     ##     ## ##     ## #### ##    ## \n");
	printf("\n\n                      [ �����Ϸ��� �ƹ�Ű�� �������� ]\n");
	getch();

	while (1)
	{
		system("cls"); // �ܼ�â �ʱ�ȭ
		printf("\n\n\n                            [ ���θ޴� ]\n\n\n\n\n"); // ���θ޴�
		printf("                            1. ���ӽ���\n\n");
		printf("                            2. ��Ϻ���\n\n");
		printf("                            3. �� �� ��\n\n");
		printf("                            4. ��������\n\n");
		printf("                       ����>");
		scanf("%d", &choice);

		switch (choice)
		{
		case 1:
			gamemain(); // ������ �� �Լ� ȣ�� (���� ����)
			break;
		case 2:
			viewlog(); // ���� ��� ��� �Լ� ȣ��
			break;
		case 3:
			help(); // ���� ��� �Լ� ȣ��
			break;
		case 4:
			system("cls"); // �ܼ�â �ʱ�ȭ
			return 0; // ���� ����
			break;
		default: // �׿� �Է� ����
			break;
		}
	}
}

void help() // ���� ��� �Լ�
{
	system("cls"); // �ܼ�â �ʱ�ȭ
	printf("Ÿ�ڿ������� �� ��վ��� �߾��� �� ���� '�꼺��'�� C���� ������ �����Դϴ�.\n");
	printf("�ϴÿ��� ������ �ܾ���� ���� �Է��Ͽ� �����ּ���!\n");
	printf("�ϴ��� ���� �̿� ��(pH)�� 0�� �Ǹ� ������ ����˴ϴ�.\n");
	printf("������ ����˴ϴ�. ���θ޴��� '2. ��Ϻ���'���� Ȯ�� �����մϴ�.\n\n");
	printf("�Է��� ���� ���¿��� ���͸� ������ ȭ���� �ʱ�ȭ�Ǵ� ���װ� �ֽ��ϴ�.\n\n\n"); // ����
	printf("�ƹ�Ű�� ������ ���� �޴��� �̵��մϴ�.");
	getch();
}

void viewlog() // ���� ����� ����ϴ� �Լ�
{
	FILE *fp2 = NULL; // ���� ���� ������ �ҷ��� ���� ������
	double s;
	int cnt; // N��° ��ȣ�� �ޱ�
	system("cls"); // �ܼ�â �ʱ�ȭ
	printf("[ ���� ]\n\n\n");
	fp2 = fopen("score.txt", "r"); // ���� ���� �ҷ�����
	if (fp2 == NULL) // ���� ���� ����
		printf("�ѹ��� ������ �Ͻ����� ���ų�,\n���� ������ �ҷ����µ� ������ �ֽ��ϴ�.\n");
	else
	{
		cnt = 1; // ù��° ����
		while (EOF != fscanf(fp2, "%lf", &s)) // ������ ���������� �����鼭
		{
			printf("%d. %.2lf��\n", cnt, s); // ���� ���
			cnt++; // ���� ��ȣ
		}
	}
	printf("\n\n�ƹ�Ű�� ������ ���θ޴��� �̵��մϴ�.");
	getch();
}
void gamemain() // ������ �� �Լ�
{
	FILE *fp = NULL; // ���� ������ ���� ���� ������
	ph = 7.0; // ���� �̿� �� 7.0���� �ʱ�ȭ
	system("cls"); // �ܼ�â �ʱ�ȭ
	initrains(); // �ܾ� �迭 �ʱ�ȭ
	start_thread(); // ����ڷκ��� �Է��� �޴� ������ ����
	PlaySound(TEXT(BGMPATH), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP); // ������� ���
	start = clock(); // ���� ���� �ð� ���
	while (1)
	{
		Sleep(SPEED); // ������ �ð���ŭ �ܾ� ���� ����
		makerain(); // ���ο� �ܾ� ����
		end = clock(); // �ش� �ܾ ������ �ð� ���
		sec = (double)(end - start) / CLK_TCK; // ������� ������ �ð�
		prnscreen(); // ȭ�� ���
		if (ph <= 0) // ���� �̿� �󵵰� 0 �����̸�
			break; // ���� ����
	}
	PlaySound(NULL, 0, 0); // ������� ����
	end_thread(); // �Է� ������ ����
	printf("\nGame Over!\n");
	fp = fopen("score.txt", "a"); // ���� ���� ���� ����
	if (fp == NULL) // ���� ���� ����
		printf("���� ��� ���� �ۼ� ����!\n\n");
	else
	{
		fprintf(fp, "%.2lf\n", sec); // ���� ���
		fclose(fp);
	}
	printf("�ƹ�Ű�� ������ ���θ޴��� �̵��մϴ�.\n");
	printf("���θ޴��� ��Ÿ���� ������ �ѹ� �� �Է����ּ���.");
	getch();
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

void makerain() // ���ο� �ܾ�(��)�� �����ϴ� �Լ�
{
	int i;
	for (i = 20; i >= 0; i--)
	{
		strcpy(rains[i].word, rains[i - 1].word); // ���� �ܾ�� �� �پ� �а�
		rains[i].x = rains[i - 1].x;
		rains[i - 1].x = 0;
		strcpy(rains[i - 1].word, " "); // �� ���� ����ó��
	}
	rains[0].x = rand() % 53;
	srand(time(NULL));
	strcpy(rains[0].word, words[rand() % 35]); // ���ο� �ܾ �������� ��ġ
}

void initrains() // �ܾ� �迭 �ʱ�ȭ �Լ� (��� ��������)
{
	int i;
	for (i = 0; i < 21; i++)
	{
		rains[i].x = 0;
		strcpy(rains[i].word, " ");
	}
}

void *t_function(void *data) // ������ ó���� �ܾ� �Է� �Լ�
{
	while (!thr_exit) // �����尡 ������ ������ �Է��� ��� ����
	{
		int i;
		gets(buffer);
		for (i = 0; i < 20; i++)
		{
			if (strstr(rains[i].word, buffer)) // �Է� ���� ��ġ�ϴ� �ܾ ������
				strcpy(rains[i].word, " "); // �ش� �ܾ� ����
		}
	}
}

void start_thread() // ������ ���� �Լ�
{
	thr_exit = 0;
	thr_id = pthread_create(&p_thread, NULL, t_function, NULL); // ������ ����
}

void end_thread() // ������ ���� �Լ�
{
	thr_exit = 1;
	pthread_cancel(p_thread); // ������ ���
}
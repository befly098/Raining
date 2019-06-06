#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<time.h>
 
#define BUF_SIZE 100
#define MAX_CLNT 3
#define MAX_IP 30

void * give_word();
void * handle_clnt(void *arg);
void send_msg(char *msg, int len);
void error_handling(char *msg);
void menu(char port[]); 
 
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
int start_flag=0;
pthread_mutex_t mutx;
pthread_mutex_t cannot_mutx;

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id,tt_id;
 
    /** time log **/
    struct tm *t;
    time_t timer = time(NULL);
    t=localtime(&timer);
 
    if (argc != 2)
    {
        printf(" Usage : %s <port>\n", argv[0]);
        exit(1);
    }
 
    menu(argv[1]);
 
    pthread_mutex_init(&mutx, NULL);
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));
 
    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
        error_handling("bind() error");
    if (listen(serv_sock, 5)==-1)
        error_handling("listen() error");

    pthread_create(&tt_id,NULL,give_word,NULL); 
    while(1)
    {
	if(clnt_cnt<MAX_CLNT){
		t=localtime(&timer);
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	 
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf(" Connceted client IP : %s ", inet_ntoa(clnt_adr.sin_addr));
		printf("(%d-%d-%d %d:%d)\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday,
		t->tm_hour, t->tm_min);
		printf(" player (%d/%d)\n", clnt_cnt,MAX_CLNT);

	}
	if(clnt_cnt==MAX_CLNT){
		start_flag=1;
	}
	
    }


    pthread_join(tt_id,NULL);
    pthread_join(t_id,NULL);
    close(serv_sock);
    return 0;
}

void *give_word(){
    char msg[BUF_SIZE]={0};
    int i,j;
    for(i=0;i<100;i++){
    	while(start_flag==0);
	sleep(1);
	sprintf(msg,"%d from server\n",i);
	msg[sizeof(msg)]='\0';
	for(j=0;j<MAX_CLNT;j++)
		write(clnt_socks[j],msg,sizeof(msg));
	msg[0]='\0';
    }
}

void *handle_clnt(void *arg)
{
    int clnt_sock=*((int*)arg);
    int str_len=0, i=0;
    char msg[BUF_SIZE]={0};
 

    while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
	{
	msg[str_len]='\0';
        send_msg(msg, str_len);
	i++;
	msg[0]='\0';
	}
 
    // remove disconnected client
    pthread_mutex_lock(&mutx);
    for (i=0; i<clnt_cnt; i++)
    {
        if (clnt_sock==clnt_socks[i])
        {
            while(i++<clnt_cnt-1)
                clnt_socks[i]=clnt_socks[i+1];
	    printf("ddds");
            break;
        }
    }
    clnt_cnt--;
    start_flag=0;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}
 
void send_msg(char* msg, int len)
{
    int i;
    pthread_mutex_lock(&mutx);
    for (i=0; i<clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
    msg[0]='\0';
    pthread_mutex_unlock(&mutx);
}
 
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
 
 
void menu(char port[])
{
    system("clear");
    printf(" **** moon/sun chat server ****\n");
    printf(" server port    : %s\n", port);
    printf(" max connection : %d\n", MAX_CLNT);
    printf(" ****          Log         ****\n\n");
}

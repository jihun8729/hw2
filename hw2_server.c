#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 6060
void error_handling(char *message);
typedef struct{
        int seq_no;
        char buffer[BUF_SIZE];
}info;

int main(int argc, char *argv[])
{
	int serv_sock;
	char message[BUF_SIZE];
	int str_len;
	socklen_t clnt_adr_sz;
	info recv;
	struct sockaddr_in serv_adr, clnt_adr;

	FILE * fp;
	fp = fopen("test.jpg","wb");
	
	int send_no;
	
	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	
	if (serv_sock == -1)
		error_handling("UDP socket creation error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	int read_cnt=0;
	if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");

		clnt_adr_sz = sizeof(clnt_adr);
		while(1){
			recvfrom(serv_sock, &recv, sizeof(recv), 0, 
				(struct sockaddr*)&clnt_adr, &clnt_adr_sz); // seq_no와 내용이 있는 구조체 수신
			
			if(recv.seq_no==-1){
				break;
			}

			read_cnt = fwrite((void*)recv.buffer,1,BUF_SIZE,fp); //내용을 버퍼크기만큼 파일에 작성
			send_no = recv.seq_no; 
			sendto(serv_sock,&send_no,sizeof(int),0,(struct sockaddr*)&clnt_adr, clnt_adr_sz);
		}
	
	fclose(fp);
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

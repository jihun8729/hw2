#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define BUF_SIZE 6060
void error_handling(char *message);

typedef struct{
        int seq_no;
        char buffer[BUF_SIZE];
}info;


int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    int recv_no;
    int cansend=1;
    double throughput;
    int recv_cnt=0;
    int read_cnt =0; 
    int count =0;
    int data_size;
    socklen_t adr_sz;
    info sender;
    struct timeval start, end;
    struct sockaddr_in serv_adr, from_adr;
    FILE *fp;
    fp = fopen("test2.jpg","rb");

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    struct timeval optval= {3,0};// 소켓 옵션으로 timeout 지정
    int optlen = sizeof(optval);
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&optval,optlen); 

    if (sock == -1)
    error_handling("socket() error");
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_adr.sin_port=htons(atoi(argv[2]));

    
    gettimeofday(&start,NULL);
    while(1){
        if(cansend==1){ //전송가능 상태
            memset(message,0,BUF_SIZE); //버퍼 초기화
            memset(sender.buffer,0,BUF_SIZE); //버퍼 초기화
            
            read_cnt = fread((void*)message, 1, BUF_SIZE, fp); //파일에서 버퍼사이즈만큼 읽기
            
            if(read_cnt<=0) break; 
            
            memcpy(sender.buffer, message, read_cnt); // 서버에 전송할 구조체에 내용 복사
            sender.seq_no = count; // seq번호 저장
            count++; // seq보낸 개수 증가
            sendto(sock, &sender, sizeof(info), 0, 
                    (struct sockaddr*)&serv_adr, sizeof(serv_adr)); //전송
            data_size+= read_cnt;        
            printf("%d: 전송\n",sender.seq_no);
        }

        adr_sz = sizeof(from_adr);
        recv_cnt=recvfrom(sock, &recv_no, sizeof(int), 0, 
            (struct sockaddr*)&from_adr, &adr_sz); // ack수신

        if(recv_cnt==-1){ //time out 경우
            printf("%d: 재전송\n",sender.seq_no);
            sendto(sock, &sender, sizeof(info), 0, 
                (struct sockaddr*)&serv_adr, sizeof(serv_adr)); //동일한 내용 재전송
            cansend=0; //전송불가능 상태로 전환
            continue;
        }

        if(recv_no==sender.seq_no){ //수신한 ack의 seq_no가 동일한경우 성공적으로 전송완료
            printf("%d: 성공\n",recv_no);
            cansend=1; // 다시 전송가능상태로 전환
        }
        else{ // ack를 수신하지 못하는경우
            printf("답장 x\n");
            cansend=0;
        }
    }
    fclose(fp);
     gettimeofday(&end,NULL);
    sender.seq_no=-1;
    sendto(sock, &sender, sizeof(info), 0, //파일을 다 전송하고 나서 종료를 위해 -1번을 전송
        (struct sockaddr*)&serv_adr, sizeof(serv_adr));

    throughput = data_size / ( (end.tv_sec-start.tv_sec)+(end.tv_usec-start.tv_usec)/ 1000000.0);
    printf("throughput : %f\n",throughput);
    close(sock);
    return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/* Program to emulate the Mikrotik Bandwidth test protocol */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define BTEST_PORT 2000
#define BTEST_PORT_CLIENT_OFFSET 256
#define CMD_PROTO_UDP 0
#define CMD_PROTO_TCP 1
#define CMD_DIR_RX 1
#define CMD_DIR_TX 2
#define CMD_DIR_TXRX 3
#define CMD_RANDOM 0

unsigned char helloStr[] = { 0x01, 0x00, 0x00, 0x00 };
unsigned char cmdStr[16];
unsigned int udpport=BTEST_PORT;

struct cmdStruct {
	int proto;
	int direction;
	int random;
	int tcp_conn_count;
	int tx_size;
	int unknown;
	int remote_tx_speed;
	int local_tx_speed;
};

int server();
int server_conn(int cmdsock, char *);
struct cmdStruct unpackCmdStr(unsigned char *);
int test_udp(struct cmdStruct, int, char *); 
int test_tcp(struct cmdStruct);

int main(){
	server();
}

int server() {
	int controlSocket;
	char buffer[1024];
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	socklen_t addr_size;
	int newSocket;

	controlSocket = socket(PF_INET, SOCK_STREAM, 0);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(BTEST_PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	bind(controlSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	if(listen(controlSocket,5)==0)
		printf("Listening\n");
	else
		printf("Error\n");
	
	addr_size = sizeof(clientAddr);
	while(1) {
		newSocket = accept(controlSocket, (struct sockaddr *) &clientAddr, &addr_size);
		/* fork a child process to handle the new connection */
		if (!fork()) {
			server_conn(newSocket, strdup(inet_ntoa(clientAddr.sin_addr)));
      			close(newSocket);
			printf("Complete\n");
      			exit(0);
		} else {
			/*if parent, close the socket and go back to listening new requests*/
			close(newSocket);
		}
	}
	return 0;
}

int server_conn(int cmdsock, char *remoteIP) {
	int nBytes = 1;
	int i;
	struct cmdStruct cmd;

	int flag = 1; 
	setsockopt(cmdsock, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

	/* Send hello message */
        send(cmdsock,helloStr,sizeof(helloStr),0);

	/* Recieve command */
	nBytes=recv(cmdsock,cmdStr,sizeof(cmdStr),0);
	if (nBytes < sizeof(cmdStr)) {
		return(-1);
	}

	cmd=unpackCmdStr(cmdStr);
	printf("proto=%d\n" , cmd.proto);
	printf("direction=%d\n" , cmd.direction);
	printf("random=%d\n" , cmd.random);
	printf("tcp_conn_count=%d\n" , cmd.tcp_conn_count);
	printf("tx_size=%d\n" , cmd.tx_size);
	printf("unknown=%d\n" , cmd.unknown);
	printf("remote_tx_speed=%d\n" , cmd.remote_tx_speed);
	printf("local_tx_speed=%d\n" , cmd.local_tx_speed);
	printf("remoteIP=%s\n" , remoteIP);

	/* Send all OK message */
        send(cmdsock,helloStr,sizeof(helloStr),0);
	
	if (cmd.proto == CMD_PROTO_UDP) {
		test_udp(cmd, cmdsock, remoteIP);
	} else {
		test_tcp(cmd);
	}
	/*
	/*loop while connection is live*/
}

struct cmdStruct unpackCmdStr(unsigned char *cmdStr) {
	struct cmdStruct cmd;

	cmd.proto=cmdStr[0];
	cmd.direction=cmdStr[1];
	cmd.random=cmdStr[2];
	cmd.tcp_conn_count=cmdStr[3];

	/* Little endian */
	cmd.tx_size  = cmdStr[4];
	cmd.tx_size += cmdStr[5] << 8;

	/* Assume little endian */
	cmd.unknown  = cmdStr[6];
	cmd.unknown += cmdStr[7] << 8;
	
	cmd.remote_tx_speed  = cmdStr[8] << 24;
	cmd.remote_tx_speed += cmdStr[9] << 16;
	cmd.remote_tx_speed += cmdStr[10] << 8;
	cmd.remote_tx_speed += cmdStr[11];

	cmd.local_tx_speed  = cmdStr[12] << 24;
	cmd.local_tx_speed += cmdStr[13] << 16;
	cmd.local_tx_speed += cmdStr[14] << 8;
	cmd.local_tx_speed += cmdStr[15];

	return(cmd);
}

int udpSocket;

void *test_udp_tx(void *arg) {
	struct cmdStruct *pcmd;
	unsigned char *buf;
	int seq=1;
	int tmp, i;

	printf("Calling test_udp_tx()\n");
	sleep(1);
	pcmd = (struct cmdStruct *)arg;
	printf("Calling test_udp_tx(%d)\n", pcmd->tx_size);
	buf=(unsigned char *) malloc(pcmd->tx_size-28);
	printf("Calling test_udp_tx(more)\n");
	bzero(buf, sizeof(buf));
	while(1) {
		int tmp=seq++;
		int nBytes;
	
		//printf("seq=%d\n", seq);
		/* Put in sequence number */
		for (i=3; i>=0; i--) {
			buf[i]=tmp % 256;
			tmp = tmp >> 8;
		}
		send(udpSocket, buf, pcmd->tx_size-28,0);
		/*
		if (send(udpSocket, buf, pcmd->tx_size-28,0)<0) {
			perror("send udp: ");
			exit(-1);
		}
		*/
	}
}

int test_udp(struct cmdStruct cmd, int cmdsock, char *remoteIP) {
	unsigned char socknumbuf[2];	
	pthread_t pth_tx;
	pthread_t pth_rx;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addr_size;
	int nBytes, i;
	char buffer[1024];

	udpport++;
	printf("Calling test_udp(udpport=%d)\n", udpport);
	/* Send server socket number */
	socknumbuf[0]=udpport / 256;
	socknumbuf[1]=udpport % 256;

	/* Send server socket number */
        send(cmdsock,socknumbuf,sizeof(socknumbuf),0);

	addr_size = sizeof(clientAddr);

	/* Create a UDP socket to transmit/recieve the data */
	udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(udpport);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(udpport+BTEST_PORT_CLIENT_OFFSET);
	clientAddr.sin_addr.s_addr = inet_addr(remoteIP);
	memset(clientAddr.sin_zero, '\0', sizeof clientAddr.sin_zero);  

	/* Connect it to the remote end */
	connect(udpSocket, (struct sockaddr *) &clientAddr, sizeof(clientAddr));

	if (cmd.direction == CMD_DIR_TX) {
		pthread_create(&pth_tx,NULL,test_udp_tx,(void *)&cmd);
	}

	printf("Listening on TCP cmdsock\n");
	while(nBytes=recv(cmdsock,buffer,1024,0)){
		for (i=0;i<nBytes-1;i++){
			printf("%02x", buffer[i]);
		}
		printf("\n");
        }
}

int test_tcp(struct cmdStruct cmd) {
}


#include "header.h"

char* receiveMsgFromServer(int sockFD) {
	int numPacketsToReceive = 0;
	int n = read(sockFD, &numPacketsToReceive, sizeof(int));
	if(n <= 0) {
		shutdown(sockFD, SHUT_WR);
		return NULL;
	}
	char *str = (char*)malloc(numPacketsToReceive*512);
	memset(str, 0, numPacketsToReceive*512);
	char *str_p = str;
	int i;
	for(i = 0; i < numPacketsToReceive; ++i) {
		int n = read(sockFD, str, 512);
		str = str+512;
	}
	return str_p;
}

void sendMsgToServer(int sockFD, char *str) {
	int numPacketsToSend = (strlen(str)-1)/512+ 1;
	int n = write(sockFD, &numPacketsToSend, sizeof(int));
	char *msgToSend = (char*)malloc(numPacketsToSend*512);
	strcpy(msgToSend, str);
	int i;
	for(i = 0; i < numPacketsToSend; ++i) {
		int n = write(sockFD, msgToSend, 512);
		msgToSend += 512;
	}
}

int main(int argc, char **argv) {
	int sockFD, portNO;
	struct sockaddr_in serv_addr;
	char *msgFromServer;
	char msgToServer[257];

	if(argc < 3) {
		fprintf(stderr, "Usage: %s host_addr port_number\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	portNO = atoi(argv[2]);
	if((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error in opening socket.\n");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portNO);
	if((inet_aton(argv[1], &serv_addr.sin_addr)) == 0) {
		exit(EXIT_FAILURE);
	}

	if(connect(sockFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		exit(EXIT_FAILURE);
	}

	printf("Connected to Server.\n");

	for(int i=1;;i++){
		msgFromServer = receiveMsgFromServer(sockFD);
		if(msgFromServer == NULL)
			break;
		if(strncmp(msgFromServer, "unauth", 6) == 0) {
			printf("Unautherized User.\n");
			shutdown(sockFD, SHUT_WR);
			break;
		}
		printf("%s",msgFromServer);
		printf("\n");
		free(msgFromServer);
		

		memset(msgToServer, 0, sizeof(msgToServer));
		scanf("%s", msgToServer);
		sendMsgToServer(sockFD, msgToServer);
		if(strncmp(msgToServer, "exit", 4) == 0) {
			shutdown(sockFD, SHUT_WR);
			break;
		}
	}

	for(int i=1;;i++){
		msgFromServer = receiveMsgFromServer(sockFD);
		if(msgFromServer == NULL)
			break;
		printf("%s",msgFromServer);
		printf("\n");
		free(msgFromServer);
	}
	shutdown(sockFD, SHUT_RD);
	printf("Connection closed\n");
	return 0;
}

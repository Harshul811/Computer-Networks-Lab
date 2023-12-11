#include "header.h"

#define MLEN 256
#define MAX_LINES_IN_MS 5
#define CUSTOMER 0
#define POLICE 1
#define ADMIN 2
#define UNAUTH_USER -1

#define RBYTES 512

struct userInfo{
	char userId[MLEN+1];
	char pass[MLEN+1];
};


void sendMsgToClient(int clientFD, char *str) {
	int numPacketsToSend = (strlen(str)-1)/RBYTES + 1;
	int n = write(clientFD, &numPacketsToSend, sizeof(int));
	char *msgToSend = (char*)malloc(numPacketsToSend*RBYTES);
	strcpy(msgToSend, str);
	int i;
	for(i = 0; i < numPacketsToSend; ++i) {
		int n = write(clientFD, msgToSend, RBYTES);
		msgToSend += RBYTES;
	}
}

char* receiveMsgFromClient(int clientFD) {
	int numPacketsToReceive = 0;
	int n = read(clientFD, &numPacketsToReceive, sizeof(int));
	if(n <= 0) {
		shutdown(clientFD, SHUT_WR);
		return NULL;
	}
	
	char *str = (char*)malloc(numPacketsToReceive*RBYTES);
	memset(str, 0, numPacketsToReceive*RBYTES);
	char *str_p = str;
	int i;
	for(i = 0; i < numPacketsToReceive; ++i) {
		int n = read(clientFD, str, RBYTES);
		str = str+RBYTES;
	}
	return str_p;
}

struct userInfo getUserInfo(int clientFD) {
	int n;
	char *username = "Enter Username:";
	char *password = "Enter Password:";
	char *buffU;
	char *buffP;

	sendMsgToClient(clientFD, username);
	buffU = receiveMsgFromClient(clientFD);

	sendMsgToClient(clientFD, password);
	buffP = receiveMsgFromClient(clientFD);

	struct userInfo uInfo;
	memset(&uInfo, 0, sizeof(uInfo));
	int i;
	for(i = 0; i < MLEN; ++i) {
		if(buffU[i] != '\n' && buffU[i] != '\0') {
			uInfo.userId[i] = buffU[i];
		} else {
			break;
		}
	}
	uInfo.userId[i] = '\0';

	for(i = 0; i < MLEN; ++i) {
		if(buffP[i] != '\n' && buffP[i] != '\0') {
			uInfo.pass[i] = buffP[i];
		} else {
			break;
		}
	}
	uInfo.pass[i] = '\0';
	if(buffU != NULL)
		free(buffU);
	buffU = NULL;
	if(buffP != NULL)
		free(buffP);
	buffP = NULL;
	return uInfo;
}

char* readFromFile(FILE *fp) {
	fseek(fp, 0, SEEK_END);	
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if(sz == 0)
		return NULL;
	char *str = (char *)malloc((sz+1)*sizeof(char));
	fread(str, sizeof(char), sz, fp);
	str[sz] = 0;
	return str;
}

int authorizeUser(struct userInfo uInfo) {
	FILE *fp = fopen("login_file", "r");
	char delim[] = ", \n";
	char *str = readFromFile(fp);
	fclose(fp);
	char *save_ptr;
	char *tok = strtok_r(str, delim, &save_ptr);
	do {
		if(strcmp(uInfo.userId, tok) == 0) {
			tok = NULL;
			tok = strtok_r(NULL, delim, &save_ptr);
			if(strcmp(uInfo.pass, tok) == 0) {
				tok = NULL;
				tok = strtok_r(NULL, delim, &save_ptr);
				if(strcmp(tok, "C") == 0)
					return CUSTOMER;	
				else if(strcmp(tok, "A") == 0)
					return ADMIN;
				else if(strcmp(tok, "P") == 0)
					return POLICE;
			}
		} else {
			tok = strtok_r(NULL, delim, &save_ptr);
			tok = strtok_r(NULL, delim, &save_ptr);
		}
		tok = NULL;
	} while((tok = strtok_r(NULL, delim, &save_ptr)) != NULL);
	if(str!=NULL)
		free(str);
	return UNAUTH_USER;
}

void addStrings(char** str1, const char* str2,char del)
{
    size_t len1 = *str1 ? strlen(*str1) : 0;
    size_t len2 = str2 ? strlen(str2) : 0;
    char *res = realloc(*str1, len1 + len2 + 2);
    if (res)
    {
        res[len1] = del;
        memcpy(res + len1 + 1, str2, len2);
        res[len1 + 1 + len2] = 0;
        *str1 = res;
    }
}

void printMiniStatement(int clientFD, char *fileName) {
	FILE *fp = fopen(fileName, "r");
	char delim[] = "\n";
	char *str = readFromFile(fp);
	fclose(fp);

	char *miniSt = NULL;
	char *tok = strtok(str, delim);
	int cnt = 0;
	do {
		if(cnt == 0 && tok != NULL) {
			miniSt = (char*)malloc(((strlen(tok)+1))*sizeof(char));
			strcpy(miniSt, tok);
			miniSt[strlen(tok)] = 0;
		}
		else
			addStrings(&miniSt, tok, '\n');
		tok = NULL;
		cnt++;
	} while((tok = strtok(NULL, delim)) != NULL && cnt < MAX_LINES_IN_MS);
	if(str!=NULL)
		free(str);
	if(miniSt == NULL)
		sendMsgToClient(clientFD, "None");
	else
		sendMsgToClient(clientFD, miniSt);
	if(miniSt != NULL)
		free(miniSt);
	miniSt = NULL;
	str = NULL;
}

char* returnBalance(char *fileName) {
	FILE *fp = fopen(fileName, "r");
	char delim[] = ",\n";
	char *str = readFromFile(fp);
	fclose(fp);

	char *save_ptr;

	char *bal = (char*)malloc(2*sizeof(char));
	bal[0] = '0';
	bal[1] = 0;
	char *tok = strtok_r(str, delim, &save_ptr);
	int cnt = 0;
	do {
		if(cnt == 2) {
			bal = (char*)malloc(((strlen(tok)+1))*sizeof(char));
			strcpy(bal, tok);
			bal[strlen(tok)] = 0;
		}
		tok = NULL;
		cnt++;
	} while((tok = strtok_r(NULL, delim, &save_ptr)) != NULL && cnt < 3);
	if(str!=NULL)
		free(str);
	str = NULL;
	return bal;
}


void processUserRequests(int clientFD, struct userInfo uInfo) {
	int n;
	char *buff = NULL;
	sendMsgToClient(clientFD, "Enter 1 for Mini-Statement, 2 for Available Balance, 3 to exit");
    for(int i=0;;i++){
		if(buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "1") == 0) {
			printMiniStatement(clientFD, uInfo.userId);
		} else if(strcmp(buff, "2") == 0) {
			char *bal = returnBalance(uInfo.userId);
			sendMsgToClient(clientFD, bal);
			free(bal);
			bal = NULL;
		} else if(strcmp(buff, "3") == 0) {
			break;
		} else {
			sendMsgToClient(clientFD, "Unknown Query");
		}
	}
	if(buff != NULL)
		free(buff);
	buff = NULL;
}

void printBalanceOfAllUsers(int clientFD) {
	FILE *fp = fopen("login_file", "r");
	char delim[] = ", \n";
	char *str = readFromFile(fp);
	fclose(fp);
	char *miniSt = (char*)malloc((strlen("User  Avail. Balance")+1)*sizeof(char));
	strcpy(miniSt, "User  Avail. Balance");
	miniSt[strlen("User  Avail. Balance")] = 0;
	char *save_ptr;
	char *tok = strtok_r(str, delim, &save_ptr);
	do {
		char *probableFileName = (char*)malloc(((strlen(tok)+1))*sizeof(char));
		strcpy(probableFileName, tok);
		probableFileName[strlen(tok)] = 0;
		tok = strtok_r(NULL, delim, &save_ptr);
		tok = strtok_r(NULL, delim, &save_ptr);
		if(strcmp(tok, "C") == 0) {
			char *bal = returnBalance(probableFileName);
			addStrings(&miniSt, probableFileName, '\n');
			addStrings(&miniSt, bal, ' ');
			free(bal);
			bal = NULL;
		}
		free(probableFileName);
		probableFileName = NULL;
		tok = NULL;
	} while((tok = strtok_r(NULL, delim, &save_ptr)) != NULL);
	if(str!=NULL)
		free(str);
	if(miniSt != NULL)
		sendMsgToClient(clientFD, miniSt);
	else
		sendMsgToClient(clientFD, "No User in bank");
	if(miniSt!=NULL)
		free(miniSt);
	miniSt = NULL;
}

void processPoliceRequests(int clientFD, struct userInfo userId) {
	int n;
	char *buff = NULL;
	sendMsgToClient(clientFD, "Enter 1 for Balance of all users, 2 for Mini Statement of User, and 3 to exit");

	for(int i=0;;i++){
		if(buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "1") == 0) {
			printBalanceOfAllUsers(clientFD);
		} else if(strcmp(buff,"2")==0){
			sendMsgToClient(clientFD, "Enter User_ID:");
            		char* buff1 = NULL;
            		buff1 = receiveMsgFromClient(clientFD);
            		printMiniStatement(clientFD,buff1);
            		break;
		}
		else if(strcmp(buff, "3") == 0) {
			break;
		} else {
			sendMsgToClient(clientFD, "Unknown Query");
		}
	}
	if(buff != NULL) free(buff);
	buff = NULL;
}

int checkIfUserExists(char *userName) {
	FILE *fp = fopen("login_file", "r");
	char delim[] = ", \n";
	char *str = readFromFile(fp);
	fclose(fp);

	char *save_ptr;
	char *tok = strtok_r(str, delim, &save_ptr);
	do {
		int check = 0;
		if(tok!=NULL && strcmp(tok, userName) == 0) {
			check = 1;
		}
		tok = strtok_r(NULL, delim, &save_ptr);
		tok = strtok_r(NULL, delim, &save_ptr);
		if(strcmp(tok, "C") == CUSTOMER && check == 1)
			return 1;
		tok = NULL;
	} while((tok = strtok_r(NULL, delim, &save_ptr)) != NULL);
	if(str!=NULL)
		free(str);
	return 0;
}

int getUserName(int clientFD, char **userName) {
	int n;
	char *buff=NULL;
	int toRet = -1;
	sendMsgToClient(clientFD, "Enter Username of the account to Credit or Debit into and 2 to terminate");
	while(1) {
		if(buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "2") == 0) {
			toRet = -1;
			break;
		} else if(checkIfUserExists(buff)) {
			*userName = (char*)malloc((n+1)*sizeof(char));
			strcpy(*userName, buff);
			toRet = 1;
			break;
		} else {
			sendMsgToClient(clientFD, "Unknown user");
		}
	}
	if(buff != NULL)
		free(buff);
	buff = NULL;
	return toRet;
}

int getQuery(int clientFD) {
	int n;
	char *buff=NULL;
	int toRet = -1;
	sendMsgToClient(clientFD, "Enter 1 to credit, 2 to debit and 3 to terminate");
	while(1) {
		if(buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "3") == 0) {
			toRet = -1;
			break;
		} else if(strcmp(buff, "1") == 0) {
			toRet = 10;
			break;
		} else if(strcmp(buff, "2") == 0) {
			toRet = 11;
			break;
		} else {
			sendMsgToClient(clientFD, "Unknown Query");
		}
	}
	if(buff!=NULL)
		free(buff);
	buff=NULL;
	return toRet;
}

int isANumber(char *num) {
	int i = 0;
	int check = 0;
	for(i = 0; i < strlen(num); ++i) {
		if(isdigit(num[i])) {
			continue;
		}
		else if(num[i] == '.' && check == 0) {
			check = 1;
		}
		else {
			return 0;
		}
	}
	return 1;
}

double getAmount(int clientFD) {
	int n;
	double toRet = -1;
	char *buff=NULL;
	sendMsgToClient(clientFD, "Enter Numerical Amount and 2 to terminate");
	while(1) {
		if(buff!=NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "2") == 0) {
			toRet = -1.0;
			break;
		} else if(isANumber(buff)) {
			toRet = strtod(buff, NULL);
			if(toRet < 0.0f)
				sendMsgToClient(clientFD, "Negative Amount Not Allowed");
			break;
		} else {
			sendMsgToClient(clientFD, "Invalid amount");
		}
	}
	if(buff!=NULL)
		free(buff);
	buff=NULL;
	return toRet;
}

void updateUserTransFile(char *fileName, int toCorD, double amount, double curBal) {
	FILE *fp = fopen(fileName, "r");
	char *str = readFromFile(fp);
	fclose(fp);
	if(str == NULL) {
		str = "";
	}
	char c_d;
	if(toCorD == 10) {
		curBal += amount;
		c_d = 'C';
	}
	else if(toCorD == 11) {
		curBal -= amount;
		c_d = 'D';
	}

	time_t ltime;
    ltime=time(NULL);

	char *data = (char*)malloc((1 + strlen(asctime(localtime(&ltime))) + 1000 + strlen(str))*sizeof(char));
	sprintf(data, "%.*s,%c,%f\n%s", (int)strlen(asctime(localtime(&ltime)))-1, asctime(localtime(&ltime)), c_d, curBal, str);

	fp = fopen(fileName, "w");
	fwrite(data, sizeof(char), strlen(data), fp);
	fclose(fp);
}

int showInSuffBal(int clientFD) {
	int n;
	int toRet = -1;
	char *buff=NULL;
	sendMsgToClient(clientFD, "Insufficient Balance. Do you want to continue?[Y/N]");
	while(1) {
		if(buff!=NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "N") == 0) {
			toRet -1;
			break;
		} else if(strcmp(buff, "Y") == 0) {
			toRet = 1;
			break;
		} else {
			sendMsgToClient(clientFD, "Unknown Query");
		}
	}
	if(buff!=NULL)
		free(buff);
	buff=NULL;
	return toRet;
}

void processAdminRequests(int clientFD) {
	while(1) {
		char *userName;
		int ret = getUserName(clientFD, &userName);
		if(ret < 0)
			return;
		ret = getQuery(clientFD);
		if(ret < 0)
			return;
		int toCorD = ret;
		double amount  = getAmount(clientFD);
		if(amount < 0.0)
			return;
		char *bal = returnBalance(userName);
		double curBal = strtod(bal, NULL);
		free(bal);
		bal = NULL;

		if(curBal < amount && toCorD == 11) {
			ret = showInSuffBal(clientFD);
			if(ret < 0)
				return;
			else
				continue;
		}
		updateUserTransFile(userName, toCorD, amount, curBal);
	}
}

void processRequests(int uType, int clientFD, struct userInfo uInfo) {

	switch(uType)
	{
		case UNAUTH_USER:
			printf("Unautherized user.\n");
			sendMsgToClient(clientFD, "unauth");
			shutdown(clientFD, SHUT_RDWR);
			break;
		
		case CUSTOMER:
			printf("CUSTOMER.\n");
			processUserRequests(clientFD, uInfo);
			sendMsgToClient(clientFD, "Thanks!");
			shutdown(clientFD, SHUT_RDWR);
			break;
		case ADMIN:
			printf("ADMIN.\n");
			processAdminRequests(clientFD);
			sendMsgToClient(clientFD, "Thanks!");
			shutdown(clientFD, SHUT_RDWR);
			break;
		case POLICE:
			printf("POLICE.\n");
			processPoliceRequests(clientFD, uInfo);
			sendMsgToClient(clientFD, "Thanks!");
			shutdown(clientFD, SHUT_RDWR);
			break;
	}
}

int main(int argc, char **argv) {
	int sockFD, clientFD, portNO, cliSz;
	struct sockaddr_in serv_addr, cli_addr;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s port_number\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error opening socket.\n");
		exit(EXIT_FAILURE);

	}

	memset((void*)&serv_addr, 0, sizeof(serv_addr));
	portNO = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;			
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portNO);		

	if(bind(sockFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Error on binding.\n");
		exit(EXIT_FAILURE);
	}

	int reuse = 1;
	setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	if(listen(sockFD, 7) < 0) {
		perror("Error on listening.\n");
		exit(EXIT_FAILURE);
	}

	cliSz = sizeof(cli_addr);

	while(1) {
		memset(&cli_addr, 0, sizeof(cli_addr));
		if((clientFD = accept(sockFD, (struct sockaddr*)&cli_addr, &cliSz)) < 0) {
			perror("Error on accept.\n");
			exit(EXIT_FAILURE);
		}

		switch(fork()) {
			case -1:
				printf("Error in fork.\n");
				break;
			case 0: {
				close(sockFD);
				struct userInfo uInfo = getUserInfo(clientFD);
				int uType = authorizeUser(uInfo);
				processRequests(uType, clientFD, uInfo);
				exit(EXIT_SUCCESS);
				break;
			}
			default:
				close(clientFD);
				break;
		}
	}

	return 0;
}

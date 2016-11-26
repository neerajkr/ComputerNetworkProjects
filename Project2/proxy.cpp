/* Proxy Server Program in which port number is provided at the terminal */

#include <stdio.h>			/*libraries*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>		//contains definitions of data types used in system call
#include <sys/socket.h>		//definition needed for socket structures
#include <netinet/in.h>		//contains values of constants used in defition of Internet Domain Addresses
#include <iostream>
#include <cstdio>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <string>
#include <netdb.h>
#include "proxy_parse.c"
#define BUF_SIZE 8192               			//Buffer size
using namespace std;

void process_server(char *, int);				//function declarations
void send_response_header_501(void);
string date_time_header(void);

socklen_t clilen;
int sockfd, newsockfd, portno;					//variables for socket and port no
struct sockaddr_in serv_addr, cli_addr;			//data structure to store internet addresses
struct ParsedRequest *req;

void error(const char *err){      				//error msg printing
	perror(err);
	exit(1);
}

string date_time_header(void){                   // function for getting current time
	char buf[1000];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	string str = string(buf);
	str="Date: "+str+"\r\n";
	return str;
}

void send_response_header_501(void){                  //header response when GET method is not requested with http
	string http_version="HTTP/1.0 ";
	string status="501 Not Implemeted\r\n";
	string headers;
	string dateTime=date_time_header();
	string server="Server: localhost:Neeraj,IITK\r\n";
	headers=http_version+status+dateTime+server+"\r\n";
	char Headers[4024] ;
	memset(Headers, '\0', sizeof(Headers));
	std::string::size_type i;
	for( i=0;i < headers.size(); ++i) {
		Headers[i]=headers[i];
	}
	write(newsockfd,Headers,strlen(Headers));			//responding to client with error msg og "501"
}

void process_server(char * send_data, int newsockfd){	// sending request to server and response from server to client
	if (req->port == NULL){
		char p[] = "80\0";
		req -> port = p;
	}
	struct hostent *ServerName;
	struct sockaddr_in ServerAddr;
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  		//creating scoket for server
		perror("Failed to establish Socket :(\n") ;
		exit(-1)    ;
	}

	ServerName = gethostbyname(req->host);
	if (ServerName == NULL){
		printf("No such host exists\n");
		exit(-1);
	}

	int ServerPort = atoi(req -> port); 
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(ServerPort);
	bcopy((char *)ServerName -> h_addr, (char *)&ServerAddr.sin_addr.s_addr, ServerName-> h_length); 

	if (connect(sock, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr)) == -1){    //connecting with server
		printf("Can't Connect to server");
		exit(-1);
	}

	send(sock, send_data, strlen(send_data), 0);		//sending request to server
	int bytes;
	char recv_data[8194];
	bzero(recv_data, sizeof(recv_data));
	while((bytes = recv(sock, recv_data, sizeof(recv_data), 0)) > 0){
		send(newsockfd, recv_data, bytes, 0);			//sending response to client from server
		bzero(recv_data, sizeof(recv_data));
	}
}


int main(int argc, char *argv[])		//main function and port number is taken in its argument
{
	char buffer[BUF_SIZE];				//acts as a buffer
	int n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);				//sys call to create a new socket for TCP connection
	if (sockfd < 0)    error("socket creation error\n"); 	//error for unsuccesful creation of socket
	bzero((char *) &serv_addr, sizeof(serv_addr));	
	portno = atoi(argv[1]);									//extracting port number in nuerical form
	serv_addr.sin_family = AF_INET;		
	serv_addr.sin_addr.s_addr = INADDR_ANY;		
	serv_addr.sin_port = htons(portno);	

	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)	error("ERROR on binding");  

	listen(sockfd,10);										//it allows the system to listen for connections

	clilen = sizeof(cli_addr);

	static int counter=0;
	int pid;

	while(1){
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);  //blocking the process unless a client connects to server
		if ((pid = fork()) == -1){
			close(newsockfd);
			continue;
		}
		else if(pid>0){														//parent process
			close(newsockfd);
			counter++;
			continue;
		}
		else if(pid==0){													//child process
			bzero(buffer,BUF_SIZE);									//clearing the buffer to read file name from client side
			n = read(newsockfd,buffer,BUF_SIZE);
			if (n <= 0) error("ERROR reading from socket");

			req = ParsedRequest_create()  ;
			int status = ParsedRequest_parse(req, buffer, strlen(buffer))      ;
			if(status<0)	
				cout << "Parse Failed\n" << endl;

			ParsedHeader_remove(req, "Connection")           ;
			ParsedHeader_set(req, "Connection", "close")    ;

			int rlen = ParsedRequest_totalLen(req);
			char *b = (char *)malloc(rlen+1);
			if (ParsedRequest_unparse(req, b, rlen) < 0)
				printf("Failed Unparsing\n");
			b[rlen]='\0';

  			std::string str2 ("GET");
  			if (str2.compare(req->method) == 0){			//if method is GET with http protocol then send request to server
  				process_server(b, newsockfd)    ;
  			}
  			else{
  				send_response_header_501();						//else send not implementes msg
  			}
			free(b) ;
			close(newsockfd);
			break;
		}
	}
close(sockfd);
return 0;
}

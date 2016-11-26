#include <stdio.h>    //similar to server.cpp, these header files are different libraries
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdio>

void error(const char *err)   			//error message
{
    perror(err);
    exit(0);
}

int main(int argc, char *argv[])			//input the port number and ip from commandline
{
    int sockfd, portno, n;		//variables for socket and port no
    struct sockaddr_in serv_addr;	///data structure to store internet addresse	
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {		// error if number of argument is lss
       fprintf(stderr,"Enter thre arugument like ./client ip port\n");
       exit(0);
    }
    portno = atoi(argv[2]);		//get the port number from terminal
    sockfd = socket(AF_INET, SOCK_STREAM, 0);		//create the socket
    if (sockfd < 0)					// erro if there was problem on creating the socket
        error("Socket opening problem\n");
    server = gethostbyname(argv[1]);			// get the hostname either localhost or someother ip
    if (server == NULL) {
        fprintf(stderr,"No host exists\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));		//clearing the serv_addr	
    serv_addr.sin_family = AF_INET;				//assigning values to serv_addr object
    bcopy((char *)server->h_addr,      (char *)&serv_addr.sin_addr.s_addr,     server->h_length);
    serv_addr.sin_port = htons(portno);			//assigning port number to serv_addr




    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)		//connecting with server
        error("ERROR connecting");			//error if not conected

        bzero(buffer,256);
        n = read(sockfd,buffer,255);		//reading welcome msg and diplaying on the terminal
	printf("%s\n",buffer);

	while(1){
      	  printf("Enter File Name: ");			//asking file name which need to be opened
          bzero(buffer,256);
          fgets(buffer,255,stdin);		//prompts for user input
          n = write(sockfd,buffer,strlen(buffer));		//writing filename to server
          if (n < 0)
               error("filename was not written on server");
          bzero(buffer,256);
          n = read(sockfd,buffer,255);		//reading acknowledgement and length of file if file exists

          if(atoi(buffer)==-1) printf("no such file exists\n" );		//error if file doesn't exists
	  else if(atoi(buffer)==0) printf("Note:File is empty,nothing to show\n" );

          else{		//if file name is valid then reading all the data , one packet of 255 each time

                    int count=atoi(buffer);
                    for(int i=0;i<count;i++){
                            bzero(buffer,256);
                            n = read(sockfd,buffer,255);
                            printf("%s",buffer);
                            bzero(buffer,256);

                    }
          }


	}
close(sockfd);
return 0;

}

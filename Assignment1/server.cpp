/* Server Program in which port number is provided at the terminal */

#include <stdio.h>		/*libraries*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>		//contains definitions of data types used in system call
#include <sys/socket.h>		//definition needed for socket structures
#include <netinet/in.h>		//contains values of constants used in defition of Internet Domain Addresses
#include <iostream>
#include <cstdio>
#include<strings.h>
using namespace std;

void error(const char *err)		//printing error message
{
    perror(err);
    exit(1);
}

int main(int argc, char *argv[])		//main function and port number is taken in its argument
{		
		
	   socklen_t clilen;
           char buffer[256];			//acts as a buffer of size 256
           int sockfd, newsockfd, portno;	//variables for socket and port no
           
           struct sockaddr_in serv_addr, cli_addr;	//data structure to store internet addresses	
           int n;
           if (argc < 2) {
               fprintf(stderr,"Provide the port number as well\n");
               exit(1);
           }
           sockfd = socket(AF_INET, SOCK_STREAM, 0);	//sys call to create a new socket for TCP connection
           if (sockfd < 0)    error("socket creation error\n"); //error for unsuccesful creation of socket
           bzero((char *) &serv_addr, sizeof(serv_addr));	//initialises serv_addr to zero on calling bzero() function
           portno = atoi(argv[1]);			//extracting port number in nuerical form
           serv_addr.sin_family = AF_INET;		//assigning values to serv_addr which is a structure of type sockaddr_in;AF_INET is a symbolic contant
           serv_addr.sin_addr.s_addr = INADDR_ANY;		//symbolic constant INADDR_ANY, gets IP address of server machine
           serv_addr.sin_port = htons(portno);			//htons convert host byte order to network byte order

           if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)	error("ERROR on binding");  //binding socket to a address and error msg if failed to bind
		
           listen(sockfd,5);		//it allows the system to listen for connections
		
           clilen = sizeof(cli_addr);
		
           newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);  //blocking the process unless a client connects to server
           if (newsockfd < 0)           error("ERROR on accept");
	   n = write(newsockfd,"Welcome to Server Client communication\n",sizeof("Welcome to Server Client communication\n"));		//writing welcome msg
           if (n < 0) error("ERROR writing to socket");	

           while(1){		//once connected go in infinite loop for connecting with client

               bzero(buffer,256);		//clearing the buffer to read file name from client side
               n = read(newsockfd,buffer,255);
		if(n<=0){
			//printf("Error\n");
			//listen(sockfd,5);
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);  //blocking the process unless a client connects to server
           		if (newsockfd < 0)           error("ERROR on accept");
			bzero(buffer,256);
			n = write(newsockfd,"Welcome to Server Client communication\n",sizeof("Welcome to Server Client communication\n"));		//writing welcome msg
           		if (n < 0) error("ERROR writing to socket");
			n = read(newsockfd,buffer,255);		
			}
              printf("File Name You are searching For: %s\n",buffer );		//printing the file name
              if (n <= 0) error("ERROR reading from socket");
              long length;
              strtok(buffer, "\n");   //remove newline character at the end from file name
              FILE * f = fopen (buffer, "rb"); 		//open the file and give handle to file pointer f
              int count=0;		//counts the number of packets that need to be sent
              int file_len=0;		//file size	
              char str[256];
              if(f==NULL){		//if file name is inavlid or file doesn't open send error acknowledgement as "0" to client
                n = write(newsockfd,"-1",sizeof("-1"));
                if (n < 0) error("ERROR writing to socket");
              }

              else		//if file was open correctly then send the file in different chuncks
              {
                fseek (f, 0, SEEK_END);		//take the file pointer to last of text file
                file_len=ftell(f);		//find the file length

                if(file_len%255==0) count=file_len/255;	// calculate number of chunks/packets
                else  count=file_len/255+1;
 
                bzero(str,256);		
                sprintf(str, "%d", count);	//convert count into strings 

                  n = write(newsockfd,str,255);			//send that count to client
                  if (n < 0) error("ERROR writing to socket");
                  length=255;
                  fseek (f, 0, SEEK_SET);	//take file pointer to intial point
                  while (count--) {		//send file one packet at a time
                        bzero(buffer,256);
                        fread (buffer, 1, length, f);
                        //printf("%s",buffer);
                        n = write(newsockfd,buffer,length);
                        if (n < 0) error("ERROR writing to socket");
                  }
                  fclose (f);
                }
}

close(newsockfd);
close(sockfd);
return 0;
}

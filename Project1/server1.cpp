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
#include <sys/stat.h>
#include <time.h>
#include <string>

#define BUF_SIZE 8192               //Buffer size
#define conf_file "Initial.conf"
using namespace std;

char root[200], extra[200]  ;

int status_code,content_length;                   //Initialization of variables
string content_type;
socklen_t clilen;
int sockfd, newsockfd, portno;	//variables for socket and port no
struct sockaddr_in serv_addr, cli_addr;	//data structure to store internet addresses


string date_time_header(void){                        // function for getting current time

  char buf[1000];
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
  string str = string(buf);
  str="Date: "+str+"\r\n";
  std::cout << str << std::endl;
  return str;
}

size_t getFilesize(const char* filename) {                  //function for getting file size of local file
    struct stat st;
    if(stat(filename, &st) != 0) {
        return 0;
    }
    return st.st_size;
}

void file_length(FILE *file){                     //function for getting file length
  fseek (file, 0, SEEK_END);		//take the file pointer to last of text file
  content_length=ftell(file);		//find the file length
  std::cout << "content_length:" <<content_length << std::endl;
  fseek (file, 0, SEEK_SET);
}

void type_of_content(string fn){                            //function for geeting cotent type
  if(fn.substr(fn.find_last_of(".") + 1) == "html")
    content_type="Content-Type: text/html\r\n";
  else if(fn.substr(fn.find_last_of(".") + 1) == "htm")
    content_type="Content-Type: text/html\r\n";
  else if(fn.substr(fn.find_last_of(".") + 1) == "txt")
    content_type="Content-Type: text/plain\r\n";
  else if(fn.substr(fn.find_last_of(".") + 1) == "jpeg")
    content_type="Content-Type: image/jpeg\r\n";
  else if(fn.substr(fn.find_last_of(".") + 1) == "jpg")
    content_type="Content-Type: image/jpeg\r\n";
  else if(fn.substr(fn.find_last_of(".") + 1) == "gif")
    content_type="Content-Type: image/gif\r\n";
  else if(fn.substr(fn.find_last_of(".") + 1) == "pdf")
    content_type="Content-Type: Application/pdf\r\n";
  else content_type="Content-Type: text/html\r\n";
  std::cout << content_type << std::endl;
}

string status_msg(int status_code){                           //function for status msg
  switch (status_code) {
    case 200: return  "200 OK\r\n";
    case 400: return  "400 Bad Request\r\n";
    case 404: return  "404 Not Found\r\n";
    case 500: return  "500 Internal Server Error\r\n";
    case 501: return  "501 Not Implemeted\r\n";
    default: return   "200 OK\r\n";
  }
}

void send_response_header_404(void){                  //header response
  status_code=404;
  string http_version="HTTP/1.1 ";
  string status=status_msg(status_code);
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
  //printf("%s\n", Headers);
  write(newsockfd,Headers,strlen(Headers));
}

void send_response_header(void){                      //header response
  status_code=200;
  string http_version="HTTP/1.1 ";
  string status=status_msg(status_code);
  char buf[50];
  sprintf(buf, "%d", content_length);
  string ContentLength="Content-Length: "+string(buf)+"\r\n";
  string connection_typ="Connection: Keep-Alive\r\n";
  string headers;
  string server="Server: localhost:Neeraj,IITK\r\n";
  string dateTime=date_time_header();
  headers=http_version+status+dateTime+server+ContentLength+connection_typ+content_type+"\r\n";
  std::cout << content_type << std::endl;
  std::cout << headers << std::endl;
  char Headers[4024] ;
  memset(buf, '\0', sizeof(buf))  ;
  memset(Headers, '\0', sizeof(Headers));
  std::string::size_type i;
  for( i=0;i < headers.size(); ++i) {
    Headers[i]=headers[i];
  }
  //printf("%s\n", Headers);
  cout <<strlen(Headers) <<endl;
  write(newsockfd,Headers,strlen(Headers));
}

FILE * file_open(string filename){                //function to open file
  FILE *f;
  if (filename=="/") {
      f = fopen ("index.html", "rb"); 		//open the file and give handle to file pointer f
      return f;
  }
  else if(filename.back()=='/'){
    filename=filename+"index.html";
    filename.erase(0,1);
    f = fopen (filename.c_str(), "rb"); 		//open the file and give handle to file pointer f
    if(f==NULL) std::cout << "file not opened" << std::endl;
    return f;

  }
  else{
      filename.erase(0,1);
      //std::cout << filename << std::endl;
      f = fopen (filename.c_str(), "rb"); 		//open the file and give handle to file pointer f
      if(f==NULL) std::cout << "file not opened" << std::endl;
      return f;
  }
}

void error(const char *err)       //error msg printing
{
    perror(err);
    exit(1);
}

int main(int argc, char *argv[])		//main function and port number is taken in its argument
{
           char buffer[BUF_SIZE];			//acts as a buffer
           int n;
           sockfd = socket(AF_INET, SOCK_STREAM, 0);	//sys call to create a new socket for TCP connection
           if (sockfd < 0)    error("socket creation error\n"); //error for unsuccesful creation of socket
           bzero((char *) &serv_addr, sizeof(serv_addr));	//initialises serv_addr to zero on calling bzero() function
           portno = atoi(argv[1]);			//extracting port number in nuerical form
           serv_addr.sin_family = AF_INET;		//assigning values to serv_addr which is a structure of type sockaddr_in;AF_INET is a symbolic contant
           serv_addr.sin_addr.s_addr = INADDR_ANY;		//symbolic constant INADDR_ANY, gets IP address of server machine
           serv_addr.sin_port = htons(portno);			//htons convert host byte order to network byte order
           if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)	error("ERROR on binding");  //binding socket to a address and error msg if failed to bind
           listen(sockfd,10);		//it allows the system to listen for connections
           clilen = sizeof(cli_addr);

           static int counter=0;
           int pid;
   while(1){		//once connected go in infinite loop for connecting with client
              newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);  //blocking the process unless a client connects to server
              if ((pid = fork()) == -1){
                close(newsockfd);
                continue;
              }
              else if(pid>0){
                close(newsockfd);
                counter++;
                printf("here2:%d\n",pid);
                continue;

              }
              else if(pid==0){
                printf("hereChild:%d\n",pid);
                bzero(buffer,BUF_SIZE);		//clearing the buffer to read file name from client side
                n = read(newsockfd,buffer,BUF_SIZE);
                if (n <= 0) error("ERROR reading from socket");

                //printf("File Name You are searching For:\n%s",buffer );		//printing the file name

                char * pch;
                pch = strtok (buffer," ");
                string request[100];
                int req_word_no=0;
                while (pch != NULL)
                {
                  request[req_word_no]=pch;
                  req_word_no++;
                  pch = strtok (NULL, " ");
                }
                bzero(buffer,BUF_SIZE);
                FILE * f;
                string filename;
                filename=request[1];
                type_of_content(filename);
                f=file_open(filename);
                if(f==NULL){		//if file name is inavlid or file doesn't open send error acknowledgement as "0" to client
                  //n = write(newsockfd,"-1",sizeof("-1"));
                  std::cout << "In main: File is NULL\n" << std::endl;
                  send_response_header_404();
                  close(newsockfd);
                  break;
                }
                file_length(f);
                std::cout << "In main:" << content_type << std::endl;
                std::cout << "In main:\nsending Headers" << std::endl;
                send_response_header();
                fseek (f, 0, SEEK_SET);	//take file pointer to intial point
                while (!feof(f)) {		//send file one packet at a time
                        bzero(buffer,BUF_SIZE);
                        int bytes_read=fread (buffer, 1, BUF_SIZE, f);
                        int bytes_sent=send(newsockfd, buffer, bytes_read, 0)  ;
                  }
                std::cout << "In main:file has been sent\n" << std::endl;
                fclose (f);
                close(newsockfd);
                break;
              }
}
close(sockfd);
return 0;
}

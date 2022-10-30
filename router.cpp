#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

using namespace std;

/* struct fd_set {
  int fd_count;
  socket fd_array[];
}
*/

int main(int argc, char **argv){
  int sockfd = socket(AF_INET,SOCK_DGRAM,0);

  struct sockaddr_in serveraddr,clientaddr;
  serveraddr.sin_family=AF_INET;
  serveraddr.sin_port=htons(9876);
  serveraddr.sin_addr.s_addr=INADDR_ANY;

  bind(sockfd,(struct sockaddr*)&serveraddr,
       sizeof(serveraddr));

  fd_set myfds;

  // clears the set
  FD_ZERO(&myfds);

  // add sockfd file descriptor to myfds
  FD_SET(sockfd,&myfds);

  // add STDIN_FILENO descriptor to myfds
  FD_SET(STDIN_FILENO,&myfds);
  
  while(1){
    fd_set tmp=myfds;
    int nn=select(FD_SETSIZE,&tmp,NULL,NULL,NULL);
    if(FD_ISSET(sockfd,&tmp)){
	  cout << "Got something on the socket" << endl;
      socklen_t len = sizeof(clientaddr);
      char line[5000];
      int n = recvfrom(sockfd,line,5000,0,
		       (struct sockaddr*)&clientaddr,&len);
	  cout << line << endl;
    }
    if(FD_ISSET(STDIN_FILENO,&tmp)){
	  cout << "The user typed something, I better do something with it" << endl;
      char buf[5000];
      fgets(buf,5000,stdin);
	  cout << "You typed" << buf << endl;
    }
    

  }
}

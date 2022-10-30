#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>


using namespace std;

/* struct fd_set {
  int fd_count;
  socket fd_array[];
}
*/

int main(int argc, char **argv){
  int packet_sockets[16];
  struct ifaddr *ifaddr, *tmp;

  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs");
    return 1;
  }

  int i = 0;

  // Puts all packet sockets into array
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    if(tmp->ifa_addr->sa_family==AF_PACKET){
	  cout << "Interface: " << tmp->ifa_name << endl;

      if(!strncmp(&(tmp->ifa_name[3]),"eth",3)){
        cout << "Creating Socket on interface " << tmp->ifa_name << endl;

        packet_socket[i] = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if(packet_socket[i]<0){
          perror("socket");
          return 2;
        }

        if(bind(packet_socket[i],tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
          perror("bind");
        }

        i++;
      }
    }
  }

  // Creates a fd_set with all the sockets
  fd_set fds;

  FD_ZERO(&fds);

  for(int j=0;j<i;j++){
    FD_SET(packet_sockets[j], &fds);
  }

  while(1){
    fd_set tmp = fds;
    int nn=select(FD_SETSIZE, &tmp, NULL, NULL, NULL);
  }


}

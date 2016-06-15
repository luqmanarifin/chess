#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

char buffer[205];

int sockfd, portno, len;
struct sockaddr_in serv_addr;
struct hostent *server;

int main() {

  char buffer[256];
  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }
  portno = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, 
       (char *) &serv_addr.sin_addr.s_addr,
       server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    error("ERROR connecting");
  }
  
  memset(buffer, 0, sizeof(buffer));
  while(1) {
    len = read(sockfd, buffer, 255);
    if (len < 0) {
      error("ERROR reading from socket");
    }
    printf("%s\n", buffer);
    memset(buffer, 0, sizeof(buffer));
  }
  close(sockfd);
  return 0;
}
#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

const int BUFFER_SIZE = 256;
const int XINUC_PORT = 7387;
const string XINUC_ADDRESS = "xinuc.org";

void error(string str) {
  cerr << str << endl;
  exit(0);
}

char buffer[BUFFER_SIZE];
int sockfd, len;
struct sockaddr_in serv_addr;
struct hostent *server;

void initialize_socket() {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }
  server = gethostbyname(XINUC_ADDRESS.c_str());
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, 
       (char *) &serv_addr.sin_addr.s_addr,
       server->h_length);
  serv_addr.sin_port = htons(XINUC_PORT);
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    error("ERROR connecting");
  }
}

int main() {
  initialize_socket();
  bzero(buffer, BUFFER_SIZE);
  while(1) {
    int len = read(sockfd, buffer, BUFFER_SIZE);
    printf("%d: %s", strlen(buffer), buffer);
    bzero(buffer, BUFFER_SIZE);
  }
  close(sockfd);
  return 0;
}
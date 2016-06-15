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
const int BOARD_SIZE = 8;
const int UNDEF = 9;

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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void print(int x, int y, const std::string& text, bool colored) {
  if (colored) {
    printf(ANSI_COLOR_CYAN "\033[%d;%dH%s\n" ANSI_COLOR_RESET, x, y, text.c_str());
  } else {
    printf("\033[%d;%dH%s\n", x, y, text.c_str());
  }
}


int board[BOARD_SIZE][BOARD_SIZE];
const char* pion = "kqbnr";
const string drawer[] = {"H", "O", "X", "L", "+"};
char mapping[256];

void initialize_game() {
  int len = strlen(pion);
  for(int i = 0; i < len; i++) {
    mapping[pion[i]] = i;
    mapping[pion[i] - 'a' + 'A'] = ~i;
  }
}

void parse_server_response() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = UNDEF;
    }
  }
  cout << buffer << endl;
  stringstream ss(buffer);
  string buf;
  while (ss >> buf) {
    int id = mapping[buf[0]];
    int i = 8 - (buf[2] - '0');
    int j = buf[1] - 'a';
    board[i][j] = id;
  }
}

void print_board() {
  int a_corner = 5;
  int b_corner = 10;
  int a_size = 2;
  int b_size = 4;
  int kali = 0;
  for (int i = a_corner; kali < BOARD_SIZE + 1; i += a_size + 1, kali++) {
    for (int j = b_corner; j <= b_corner + (b_size + 1) * BOARD_SIZE; j++) {
      print(i, j, "-", 0);
    }
  }

  kali = 0;
  for (int i = a_corner + 1; kali < BOARD_SIZE; i += a_size + 1, kali++) {
    for (int k = 0; k < a_size; k++) {
      for (int j = 0; j <= (b_size + 1) * BOARD_SIZE; j += b_size + 1) {
        print(i + k, b_corner + j, "|", 0);
      }
    }
  }
  for(int j = 0; j < BOARD_SIZE; j++) {
    string str;
    str.push_back(j + 'a');
    print(a_corner - 1, b_corner + j * (b_size+1) + b_size / 2, str, 0);
    print(a_corner + (a_size+1) * BOARD_SIZE + 1, b_corner + j * (b_size+1) + b_size / 2, str, 0);
  }
  for(int i = 0; i < BOARD_SIZE; i++) {
    string str;
    str.push_back(BOARD_SIZE - i + '0');
    print(a_corner + i * (a_size+1) + a_size / 2, b_corner - 2, str, 0);
    print(a_corner + i * (a_size+1) + a_size / 2, b_corner + (b_size+1) * BOARD_SIZE + 2, str, 0);
  }

  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (board[i][j] != UNDEF) {
        int p = a_corner + 1 + (a_size + 1) * i;
        int q = b_corner + 2 + (b_size + 1) * j;
        bool rev = board[i][j] >= 0;
        int id = (rev? board[i][j] : ~board[i][j]);
        for(int k = 0; k < 2; k++) {
          for(int l = 0; l < 2; l++) {
            print(p + k, q + l, drawer[id], rev);
          }
        }
      }
    }
  }
}

int main() {
  initialize_socket();
  initialize_game();
  while(1) {
    bzero(buffer, BUFFER_SIZE);
    int len = read(sockfd, buffer, BUFFER_SIZE);
    if (len < 30) {
      continue;
    }
    system("clear");
    parse_server_response();
    print_board();
  }
  close(sockfd);
  return 0;
}
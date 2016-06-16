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
int ap[5][2], bp[5][2];

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

bool get_sign(int id) {
  return id >= 0;
}

int get_abs(int id) {
  return get_sign(id)? id : ~id;
}

void parse_server_response() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = UNDEF;
    }
  }
  memset(ap, -1, sizeof(ap));
  memset(bp, -1, sizeof(bp));
  cout << "Receive from server:" << endl;
  cout << buffer << endl;
  stringstream ss(buffer);
  string buf;
  while (ss >> buf) {
    int id = mapping[buf[0]];
    int i = 8 - (buf[2] - '0');
    int j = buf[1] - 'a';
    board[i][j] = id;
    int abs_id = (get_sign(id)? id : ~id);
    ap[abs_id][get_sign(id)] = i;
    bp[abs_id][get_sign(id)] = j;
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
  for (int j = 0; j < BOARD_SIZE; j++) {
    string str;
    str.push_back(j + 'a');
    print(a_corner - 1, b_corner + j * (b_size+1) + b_size / 2, str, 0);
    print(a_corner + (a_size+1) * BOARD_SIZE + 1, b_corner + j * (b_size+1) + b_size / 2, str, 0);
  }
  for (int i = 0; i < BOARD_SIZE; i++) {
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

bool valid(int i, int j) {
  return 0 <= i && i < BOARD_SIZE && 0 <= j && j < BOARD_SIZE;
}

int pt;

// ngeprint penyerangan. a -> b
void print(int a_id, int ai, int aj, int b_id, int bi, int bj) {
  int a_sign = get_sign(a_id);
  int b_sign = get_sign(b_id);
  int abs_a_id = get_abs(a_id);
  int abs_b_id = get_abs(b_id);
  char buf[30] = {};
  int len = sprintf(buf, "%c%c%d -> %c%c%d", a_sign? pion[abs_a_id] : pion[abs_a_id] - 'a' + 'A', aj + 'a', BOARD_SIZE - ai
                                           , b_sign? pion[abs_b_id] : pion[abs_b_id] - 'a' + 'A', bj + 'a', BOARD_SIZE - bi);
  print(pt++, 60, (string) buf, 0);
}

// return true if this king attacks other king
bool king(int id, bool is_need_printed) {
  int sign = get_sign(id);
  int abs_id = get_abs(id);
  int pi = ap[abs_id][sign];
  int pj = bp[abs_id][sign];
  bool checkmate = false;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int ai = pi + i;
      int aj = pj + j;
      if (!valid(ai, aj)) continue;
      if (board[ai][aj] != UNDEF && get_sign(board[ai][aj]) != sign) {
        if (is_need_printed) {
          print(id, pi, pj, board[ai][aj], ai, aj);
        }
        if (get_abs(board[ai][aj]) == 0) {
          checkmate = true;
        }
      }
    }
  }
  return checkmate;
}

// return true if this bishop/rook/queen attacks enemy's king
bool contigous_move(int id, bool is_need_printed, vector<pair<int, int>> moves) {
  int sign = get_sign(id);
  int abs_id = get_abs(id);
  int pi = ap[abs_id][sign];
  int pj = bp[abs_id][sign];
  bool checkmate = false;

  for (auto move : moves) {
    int di = move.first;
    int dj = move.second;
    int ai = pi + di;
    int aj = pj + dj;
    while (valid(ai, aj)) {
      if (board[ai][aj] != UNDEF) {
        if (get_sign(board[ai][aj]) != sign) {
          if (is_need_printed) {
            print(id, pi, pj, board[ai][aj], ai, aj);
          }
          if (get_abs(board[ai][aj]) == 0) {
            checkmate = true;
          }
        }
        break;
      }
      ai += di;
      aj += dj;
    }
  }
  return checkmate;
}

// return true if this bishop attacks enemy's king
bool bishop(int id, bool is_need_printed) {
  vector<pair<int, int>> moves = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  return contigous_move(id, is_need_printed, moves);
}

// return true if this rook attacks enemy's king
bool rook(int id, bool is_need_printed) {
  vector<pair<int, int>> moves = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
  return contigous_move(id, is_need_printed, moves);
}

// return true if this queen attacks enemy's king
// queen is basically ROOK + BISHOP moves so we just reuse their method
bool queen(int id, bool is_need_printed) {
  bool ret1 = bishop(id, is_need_printed);
  bool ret2 = rook(id, is_need_printed);
  return ret1 || ret2;
}

bool knight(int id, bool is_need_printed) {
  int sign = get_sign(id);
  int abs_id = get_abs(id);
  int pi = ap[abs_id][sign];
  int pj = bp[abs_id][sign];
  bool checkmate = false;

  vector<pair<int, int>> moves = {{1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
  for (auto iterator : moves) {
    int di = iterator.first;
    int dj = iterator.second;
    for (int it = 0; it < 2; it++) {
      int ai = pi + di;
      int aj = pj + dj;
      if (valid(ai, aj)) {
        if (board[ai][aj] != UNDEF && get_sign(board[ai][aj]) != sign) {
          if (is_need_printed) {
            print(id, pi, pj, board[ai][aj], ai, aj);
          }
          if (get_abs(board[ai][aj]) == 0) {
            checkmate = true;
          }
        }
      }
      swap(di, dj);
    }
  }
  return checkmate;
}

// const char* pion = "kqbnr";
void checkmate_and_attack_algorithm() {
  print(5, 60, "Siapa menyerang siapa", true);
  pt = 6;

  bool checkmate1 = false;
  bool checkmate2 = false;
  checkmate1 |= king(0, true);
  checkmate2 |= king(~0, true);
  checkmate1 |= queen(1, true);
  checkmate2 |= queen(~1, true);
  checkmate1 |= bishop(2, true);
  checkmate2 |= bishop(~2, true);
  checkmate1 |= knight(3, true);
  checkmate2 |= knight(~3, true);
  checkmate1 |= rook(4, true);
  checkmate2 |= rook(~4, true);

  if (checkmate1 || checkmate2) {
    print(33, 10, "CHECKMATE!!!", 0);
  } else {
    print(33, 10, "Bukan sedang dalam kondisi checkmate", 0);
  }

  int ptr = 35;
  bool valid = 1;
  if (checkmate1 && checkmate2) {
    valid = 0;
  }
  if (king(0, true)) {
    valid = 0;
  }
  if (valid) {
    print(ptr++, 10, "Konfigurasi catur ini mungkin bisa saja terjadi.", 0);
  } else {
    print(ptr++, 10, "Ini tidak mungkin terjadi! Alasan :", 0);
    if (checkmate1 && checkmate2) {
      print(ptr++, 10, "- Tidak mungkin saling skak", 0);
    }
    if (king(0, true)) {
      print(ptr++, 10, "- 2 king tidak mungkin bersebelahan", 0);
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
    checkmate_and_attack_algorithm();
  }
  close(sockfd);
  return 0;
}

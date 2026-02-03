/*
 * pong.c - UDP ping/pong server code
 *          author: Agda Laakso
 */
#include "util.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORTNO "1266"

int main(int argc, char **argv) {
  int ch;
  int nping = 1;                   // default packet count
  char *pongport = strdup(PORTNO); // default port

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    default:
      fprintf(stderr, "usage: pong [-n #pings] [-p port]\n");
    }
  }

  // pong implementation goes here.
  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *r;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = IPPROTO_UDP;

  int s = getaddrinfo(NULL, pongport, &hints, &result);

  if (s != 0) {
    printf("Error with getaddrinfo\n");
    exit(1);
  }

  int sock;
  for (r = result; r != NULL; r = r->ai_next) {
    sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol);

    if (sock == -1) {
      continue;
    }

    if (bind(sock, r->ai_addr, r->ai_addrlen) == 0) {
      break;
    }

    close(sock);
  }

  freeaddrinfo(result);

  if (r == NULL) {
    printf("Error with bind\n");
    exit(1);
  }

  for (int i = 0; i < nping; i++) {
    size_t BUF_SIZE = 1024;
    char buffer[BUF_SIZE];
    struct sockaddr_in addr;
    socklen_t length = sizeof addr;

    addr.sin_family = AF_INET;

    unsigned long ul = atoi(pongport);
    unsigned short port = (unsigned short)ul;

    addr.sin_port = port;

    ssize_t rec =
        recvfrom(sock, buffer, BUF_SIZE, 0, (struct sockaddr *)&addr, &length);

    //  if receiving failed
    if (rec == -1) {
      printf("Error with receiving\n");
      perror("recvfrom");
    }

    for (int j = 0; j < sizeof(buffer); j++) {
      int value = buffer[i] - '0';
      int new_value = value + 1;

      buffer[i] = new_value;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    printf("pong[%d]: received packet from %s \n", i, ip);

    if (sendto(sock, buffer, rec, 0, (struct sockaddr *)&addr, length) != rec) {
      printf("length: %d\n", length);
      perror("Error sending packet back\n");
    }
  }

  printf("nping: %d pongport: %s\n", nping, pongport);

  return 0;
}

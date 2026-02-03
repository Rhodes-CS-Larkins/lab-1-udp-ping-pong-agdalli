/*
 * ping.c - UDP ping/pong client code
 *          author: Agda Laakso
 */
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#define PORTNO "1266"

int main(int argc, char **argv) {
  int ch, errors = 0;
  int nping = 1;                        // default packet count
  char *ponghost = strdup("localhost"); // default host
  char *pongport = strdup(PORTNO);      // default port
  int arraysize = 100;                  // default packet size

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'h':
      ponghost = strdup(optarg);
      break;
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 's':
      arraysize = atoi(optarg);
      break;
    default:
      fprintf(stderr,
              "usage: ping [-h host] [-n #pings] [-p port] [-s size]\n");
    }
  }

  // UDP ping implemenation goes here
  char *array[arraysize];
  for (int i = 0; i < arraysize; i++) {
    array[i] = "200";
  }

  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *r;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  int n = getaddrinfo(ponghost, pongport, &hints, &result);

  if (n != 0) {
    printf("error with getaddrinfo\n");
    exit(1);
  }

  int s;
  for (r = result; r != NULL; r = r->ai_next) {
    s = socket(r->ai_family, r->ai_socktype, r->ai_protocol);

    if (s == -1) {
      continue;
    }

    break;
  }

  freeaddrinfo(result);

  if (r == NULL) {
    printf("Error with socket\n");
    exit(1);
  }

  double total = 0;
  double avg = 0;
  int buffer_size = 1024;
  char buffer[buffer_size];
  int value;

  for (int i = 0; i < nping; i++) {
    double start = get_wctime();

    // write to server
    ssize_t w = sendto(s, array, arraysize, 0, r->ai_addr, r->ai_addrlen);

    if (w == -1) {
      perror("sendto");
      exit(1);
    }

    // read from server
    ssize_t rec =
        recvfrom(s, buffer, buffer_size, 0, r->ai_addr, &(r->ai_addrlen));

    if (rec == -1) {
      perror("recvfrom");
      exit(1);
    }

    double end = get_wctime();
    double actualTime = (end - start) * 1000;
    printf("ping[%d] : round-trip time: %.3f ms\n", i, actualTime);

    total += actualTime;

    value = i;
  }

  int bytes = value * arraysize;
  avg = total / value;

  printf("time to send %d packets of %d bytes %.3f ms (%.3f avg per packet)\n",
         value, bytes, total, avg);

  printf("nping: %d arraysize: %d errors: %d ponghost: %s pongport: %s\n",
         nping, arraysize, errors, ponghost, pongport);

  close(s);
  return 0;
}

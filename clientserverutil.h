#include <arpa/inet.h> // for inet_aton()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <argp.h>

char is_big_endian();
char * htonarb (char * input, char num_bytes, char * ret);
char * ntoharb (char * input, char num_bytes, char * ret);
int read_msg_from_packet(int socket, ssize_t *msg_len, char **ret);

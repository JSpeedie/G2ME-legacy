#include <arpa/inet.h> // for inet_aton()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <argp.h>


/** Takes a socket, a 'ssize_t *' and a 'char **' which both serve as return
 * variables and performs a series of read() calls on the socket so as to read
 * a basic packet of the format sizeof(ssize_t) bytes representing the total
 * length in bytes of the packet, followed by the packet length minus
 * sizeof(ssize_t) bytes representing the message.
 *
 * \param 'socket' a socket (typically produced by socket()) from which this
 *     function will attempt to read.
 * \param '*ret_msg_len' a pointer to a ssize_t which will be updated by this
 *     function to reflect the length of the message.
 * \param '*ret_msg_len' a pointer to a 'char *' which will be modified to a
 *     malloc'd chunk of memory containing the message sent in this packet.
 *     It is left to the caller of this function to later free() this memory.
 * \return 0 upon success, a negative integer upon failure.
 */
int read_msg_from_packet(int socket, ssize_t *ret_msg_len, char **ret) {
	/* RP: Read the packet, first its packet length, then its content */
	ssize_t packet_len = 0;
	/* RP1. Parse the length of message */
	ssize_t nbytes = read(socket, &packet_len, sizeof(packet_len));
	if ((long long) nbytes < (long long) sizeof(packet_len)) {
		fprintf(stderr, "ERROR: Failed to read packet length\n");
		return -1;
	}
	/* Calculate the message length by subtracting the packet header info (just
	 * 1 ssize_t) from the total packet length */
	(*ret_msg_len) = packet_len - sizeof(ssize_t);
	/* RP2. Allocate space to read the message into */
	(*ret) = malloc((*ret_msg_len) + 1);
	if ((*ret) == NULL) {
		fprintf(stderr, "ERROR: failed to malloc for message\n");
	}
	/* RP3. Parse the message itself */
	nbytes = read(socket, (*ret), (*ret_msg_len));
	if ((long long) nbytes < (long long) (*ret_msg_len)) {
		fprintf(stderr, "ERROR: Failed to read message\n");
		return -2;
	}

	return 0;
}

#include <arpa/inet.h> // for inet_aton()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <argp.h>


/** Returns whether or not the system this function is run on is big endian
 * or little endian byte ordered.
 *
 * \return 1 if the system is big endian byte ordered, and 0 if it is little
 *     endian byte ordered.
 */
char is_big_endian() {
	short n = 1;
	/*    (char *) &n   : cast the memory address of 'n' (which is of type
	 *                    'short *') to the address of a char. This gives us a
	 *                    char pointer to the first byte of 'n'.
	 * (*((char *) &n)) : dereference our casted pointer so we can treat the
	 *                    first byte of 'n' as a 'char'. If the value of this
	 *                    char is 1, then the system this code was run on
	 *                    must be little endian.
	 */
	return (*((char *) &n) != 1);
}


/** Takes a pointer to an item in memory ('*input') that occupies an arbitrary
 * number of bytes ('num_bytes') and converts it from host byte order to
 * network byte order.
 *
 * \param '*input' a pointer to the item in memory.
 * \param 'num_bytes' the number of bytes the item occupies in memory.
 * \param '*ret' a pointer to a section of memory which will have 'num_bytes'
 *     modified by this function to contain the return value.
 * \return void.
 */
void htonarb (char * input, char num_bytes, char * ret) {
	/* Input are pointers cast to char pointers so that we can access it by the
	 * byte */

	if (is_big_endian()) {
		/* Create 'ret' with its bytes in the same order to the bytes in
		 * 'input' */
		for (int i = 0; i < num_bytes; i++) {
			ret[i] = input[i];
		}
	} else {
		/* Create 'ret' such that its bytes are in reverse order to the bytes
		 * in 'input' */
		for (int i = 0; i < num_bytes; i++) {
			ret[i] = input[(num_bytes - 1) - i];
		}
	}

	return;
}


/** Takes a pointer to an item in memory ('*input') that occupies an arbitrary
 * number of bytes ('num_bytes') and converts it from network byte order to
 * host byte order.
 *
 * \param '*input' a pointer to the item in memory.
 * \param 'num_bytes' the number of bytes the item occupies in memory.
 * \param '*ret' a pointer to a section of memory which will have 'num_bytes'
 *     modified by this function to contain the return value.
 * \return void.
 */
void ntoharb (char * input, char num_bytes, char * ret) {
	/* Input are pointers cast to char pointers so that we can access it by the
	 * byte */

	if (is_big_endian()) {
		/* Create 'ret' with its bytes in the same order to the bytes in
		 * 'input' */
		for (int i = 0; i < num_bytes; i++) {
			ret[i] = input[i];
		}
	} else {
		/* Create 'ret' such that its bytes are in reverse order to the bytes
		 * in 'input' */
		for (int i = 0; i < num_bytes; i++) {
			ret[i] = input[(num_bytes - 1) - i];
		}
	}

	return;
}


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
	ssize_t packet_len_n = 0;
	/* RP1. Parse the length of message */
	ssize_t nbytes = read(socket, &packet_len_n, sizeof(packet_len_n));
	/* Convert 'packet_len_n' to host byte order */
	ssize_t packet_len;
	ntoharb((char *) &packet_len_n, sizeof(packet_len_n), (char *) &packet_len);

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

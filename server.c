#include <arpa/inet.h> // for inet_aton()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <argp.h>

#include "clientserverutil.h"

/* Program documentation. */
static char doc[] =
	"Argp example #3 -- a program with options and arguments using argp";

/* A description of the arguments we accept. */
static char args_doc[] = "ARG1 ARG2";

/* Make sure you specify the 3rd field in each entry, otherwise argp will treat the
 * option as not taking an argument */
static struct argp_option options[] = {
	{"port",     'p',  "PORT",            0,  "The port of the server" },
	{ 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments {
	int port, port_set;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	/* Get the input argument from argp_parse, which we know is a pointer to
	 * our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key) {
		case 'p':
			sscanf(arg, "%d", &arguments->port);
			arguments->port_set = 1;
			break;
		case ARGP_KEY_ARG:
			/* Too many arguments will be handled elsewhere. */
			break;
		case ARGP_KEY_END:
			/* Too few arguments will be handled elsewhere. */
			break;
		default:
			return ARGP_ERR_UNKNOWN;
		}
	return 0;
}


/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };


int main(int argc, char **argv) {
	int server_fd, new_socket;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	// char buffer[1024] = { 0 }; // TODO: unused

	/* argp option parsing */
	struct arguments arguments;

	/* Default values. */
	arguments.port = 0;
	arguments.port_set = 0;

	/* Parse our arguments; every option seen by parse_opt will
	   be reflected in arguments. */
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	/* Check that sufficient options were given to the program for it to run */
	if (arguments.port_set == 0) {
		fprintf(stderr, "ERROR: not enough arguments were given\n");
		exit(-1);
	}

	/* Create IPv4, TCP socket */
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "ERROR: failed to create socket\n");
		exit(-1);
	}

	/* Set options for socket so it reuses the address and port */
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, \
		sizeof(opt))) {

		fprintf(stderr, "ERROR: failed to set socket to reuses address and port\n");
		exit(-1);
	}

	/* Set address information for host address */
	address.sin_family = AF_INET; // IPv4
	address.sin_port = htons(arguments.port); // Host-to-Network for port number
	address.sin_addr.s_addr = INADDR_ANY; // Set address to any available one given by the system
	/* alternatively(?): */
	/* inet_aton("63.161.169.137", &address.sin_addr); */

	/* Bind server */
	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
		fprintf(stderr, "ERROR: failed to bind socket\n");
		exit(-1);
	}

	/* Set socket as passive (one that listens) with a backlog of 3 */
	if (listen(server_fd, 3) < 0) {
		fprintf(stderr, "ERROR: failed to listen on socket\n");
		exit(-1);
	}

	while (1) {
		/* Wait for new client */
		if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
			/* New client did NOT connect successfully */
			fprintf(stderr, "ERROR: on new client connection\n");
			exit(-1);
		/* New client connected successfully */
		} else {
			printf("Notice: Received a new client!\n"); // TODO: remove
			pid_t p;
			p = fork();
			/* If fork() failed */
			if (p < 0) {
				fprintf(stderr, "ERROR: fork() failed\n");
			/* If child process... */
			} else if (p == 0) {
				int request_type = 0;
				char *argument;

				ssize_t nbytes = read(new_socket, &request_type, sizeof(request_type));
				if ((long long) nbytes < (long long) sizeof(request_type)) {
					fprintf(stderr, "ERROR: Failed to read request type info from client\n");
					exit(-1);
				}

				/* If the request is for a command that requires an argument */
				if (request_type == (int) 'h' || request_type == (int) 'R') {
					/*****/
					/* Replace code with read_msg_from_packet() */
					/*****/
					/* RA: Read the argument specified along side the request type,
					 * if the request type comes with an argument */
					unsigned int argument_len = 0;
					/* RA1. Parse the length of argument */
					nbytes = read(new_socket, &argument_len, sizeof(argument_len));
					if ((long long) nbytes < (long long) sizeof(argument_len)) {
						fprintf(stderr, "ERROR: Failed to read argument from client request\n");
						exit(-1);
					}
					/* RA2. Allocate space to read the argument into */
					argument = malloc(argument_len + 1);
					/* RA3. Parse the argument itself */
					nbytes = read(new_socket, argument, argument_len);
					if ((long long) nbytes < (long long) argument_len) {
						fprintf(stderr, "ERROR: Failed to read argument from client request\n");
						exit(-1);
					}
					/*****/
					/* End replace */
					/*****/
					argument[argument_len] = '\0';
				}

				int pc_pipe[2];
				if (pipe(pc_pipe) == -1) {
					fprintf(stderr, "ERROR: Failed to create pipe\n");
				}

				pid_t child_pid = fork();
				/* If fork() failed */
				if (child_pid < 0) {
					fprintf(stderr, "ERROR: fork() failed\n");
				/* If child process... */
				} else if (child_pid == 0) {
					/* Close the reading end of the pipe for the child
						* since it will never read */
					close(pc_pipe[0]);
					/* Redirect stdout for the program we are about to
						* exec() to the write end of the pipe we created */
					if (dup2(pc_pipe[1], STDOUT_FILENO) == -1) {
						fprintf(stderr, "ERROR: Failed to redirect stdout to pipe\n");
					}

					/* execute G2ME with the right flags */
					if (request_type == (int) 'h') {
						execl("./G2ME", "./G2ME", "-h", argument, (char *) NULL);
					} else if (request_type == (int) 'O') {
						execl("./G2ME", "./G2ME", "-O", (char *) NULL);
					} else if (request_type == (int) 'R') {
						execl("./G2ME", "./G2ME", "-R", argument, (char *) NULL);
					}

					/* If this point is reached, the execl() command failed */
					fprintf(stderr, "ERROR: execl() failed\n");
					exit(-1);
				/* If parent process... */
				} else {
					/* RFC: Read from child process the result */
					/* PFS: Prepare data received from child process for sending */

					/* RFC1: Close the writing end of the pipe for the
						* parent since it will never write */
					close(pc_pipe[1]);

					/* RFC2: Read all content from the output of the
						* child process and simultaneously prepare it to
						* be sent off to the client */
					ssize_t recv_buf_size = 2048; /* Starting array size */
					/* PFS1: Make space for a block of memory '*msg' that
						* will be the message length immediately followed by
						* '*recv_buf' */
					char * msg = (char *) malloc(sizeof(ssize_t) + recv_buf_size);
					char * recv_buf = &msg[sizeof(ssize_t)];
					ssize_t recv_len = 0; /* This represents the number of elements in the array */
					ssize_t ret = 0; /* This will be used to store how many bytes the read() call read */

					/* PFS2: Read the data from the child process into a message */
					while ((ret = read(pc_pipe[0], &recv_buf[recv_len], 1024)) > 0) {
						recv_len += ret;
						/* Enlarge the receiving buffer if it doesn't have
							* space for a full read attempt */
						if (recv_buf_size - recv_len < 1024) {
							recv_buf_size = (2 * recv_buf_size) + 1;
							msg = (char *) realloc(msg, sizeof(ssize_t) + recv_buf_size);
							if (recv_buf == NULL) {
								fprintf(stderr, "ERROR: failed to realloc msg buffer\n");
							}
						}
					}
					/* RFC3: Close the reading end of the pipe as we are
						* done reading from it */
					close(pc_pipe[0]);

					/* // TODO: remove: */
					/* /1* At this point we have a char array 'recv_buf' with 'recv_len' */
					/* * elements that we can do with as we please. For this program, we'll */
					/* * just print them *1/ */
					/* for (int i = 0; i < recv_len; i++) { */
					/* 	printf("%c", recv_buf[i]); */
					/* } */

					/* PFS3: Update the message length inside the message. */
					ssize_t packet_len = sizeof(ssize_t) + recv_len;
					/*               &(msg[0])   : get the memory address of the 0th byte of 'msg'.
					*    (ssize_t *) &(msg[0])   : cast that address (which is of type 'char *') to the address of a ssize_t.
					* (*((ssize_t *) &(msg[0]))) : dereference our casted pointer so we can treat the memory starting at the 0th byte of 'msg' as a 'ssize_t'.
					*/
					(*((ssize_t *) &(msg[0]))) = packet_len;

					/* Send the requested data to the client */
					ssize_t nbytes = write(new_socket, msg, packet_len);
					if ((long long) nbytes < (long long) packet_len) {
						fprintf(stderr, "ERROR: Failed to send all the data to the client\n");
					}

					/* Client has now been served */
				}

				free(argument);

				exit(0);
			}
		}
	}

	return 0;
}

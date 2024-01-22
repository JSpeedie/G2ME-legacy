#include <arpa/inet.h> // for inet_aton()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <argp.h>

#include "G2ME.h"
#include "clientserverutil.h"

/* Program documentation. */
static char doc[] =
	"Argp example #3 -- a program with options and arguments using argp";

/* A description of the arguments we accept. */
static char args_doc[] = "ARG1 ARG2";

/* Make sure you specify the 3rd field in each entry, otherwise argp will treat the
 * option as not taking an argument */
static struct argp_option options[] = {
	{"address",  'a',  "SERVER_ADDRESS",  0,                    "The ip address of the server" },
	{"history",  'h',  "PLAYER_NAME",     0,                    "Output a given player's outcome history"},
	{"output",   'O',  "STDOUT",          OPTION_ARG_OPTIONAL,  "Output a ranking of all players in the system" },/* This doesn't take an argument */
	{"port",     'p',  "PORT",            0,                    "The port of the server" },
	{"record",   'R',  "PLAYER_NAME",     0,                    "Output to FILE instead of standard output" },
	{ 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments {
	/* -a */
	int server_address_flag_set;
	char * server_address;
	/* -h */
	int history_flag_set;
	/* -O */
	int output_flag_set;
	/* -p */
	int port, port_flag_set;
	/* -R */
	int record_flag_set;
	/* Used for all flags that take an argument */
	char * player_name;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	/* Get the input argument from argp_parse, which we know is a pointer to
	 * our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key) {
		case 'a':
			arguments->server_address = arg;
			arguments->server_address_flag_set = 1;
			break;
		case 'h':
			arguments->player_name = arg;
			arguments->history_flag_set = 1;
			break;
		case 'O':
			arguments->output_flag_set = 1;
			break;
		case 'p':
			sscanf(arg, "%d", &arguments->port);
			arguments->port_flag_set = 1;
			break;
		case 'R':
			arguments->player_name = arg;
			arguments->record_flag_set = 1;
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
	int client_fd;
	struct sockaddr_in server_address;

	/* argp option parsing */
	struct arguments arguments;

	/* Default values. */
	arguments.server_address_flag_set = 0;
	arguments.history_flag_set = 0;
	arguments.output_flag_set = 0;
	arguments.port = 0;
	arguments.port_flag_set = 0;
	arguments.record_flag_set = 0;

	/* Parse our arguments; every option seen by parse_opt will
	 be reflected in arguments. */
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	/* Check that sufficient options were given to the program for it to run */
	if (arguments.port_flag_set == 0 || arguments.server_address_flag_set == 0) {
		fprintf(stderr, "ERROR: both the address and the port of the server must be specified\n");
		exit(-1);
	}

	/* Create IPv4, TCP socket */
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "ERROR: failed to create socket\n");
		exit(-1);
	}

	/* Set server address information for host */
	server_address.sin_family = AF_INET; // IPv4
	server_address.sin_port = htons(arguments.port); // Host-to-Network for port number
	inet_pton(AF_INET, arguments.server_address, &server_address.sin_addr);

	int status = 0;
	if ((status = connect(client_fd, (struct sockaddr *) &server_address, sizeof(server_address))) < 0) {
		/* Did NOT Connect successfully */
		fprintf(stderr, "ERROR: Failed to connect\n");
		return -1;
	/* Connected successfully */
	} else {
		int request_type = 0;
		if (arguments.history_flag_set == 1) {
			request_type = (int) 'h';
		} else if (arguments.output_flag_set == 1) {
			request_type = (int) 'O';
		} else if (arguments.record_flag_set == 1) {
			request_type = (int) 'R';
		}

		/* h flag or R flag */
		if (arguments.history_flag_set == 1 || arguments.record_flag_set == 1) {
			unsigned int player_name_len = strlen(arguments.player_name);

			/* Put all the content of the message into an unpadded chunk of memory */
			char msg[sizeof(request_type) + sizeof(unsigned int) + strlen(arguments.player_name)];
			/* First the request type */
			memcpy(&msg[0], &request_type, sizeof(request_type));
			/* then the length of the player name */
			memcpy(&msg[sizeof(request_type)], &player_name_len, sizeof(player_name_len));
			/* then the player name */
			memcpy(&msg[sizeof(request_type) + sizeof(unsigned int)], arguments.player_name, strlen(arguments.player_name));

			/* Send request to server */
			ssize_t nbytes = write(client_fd, &msg[0], sizeof(msg));
			if ((long long) nbytes < (long long) sizeof(msg)) {
				fprintf(stderr, "ERROR: Failed to send all the data to server\n");
				exit(-1);
			}

			char *response = NULL;
			ssize_t response_len = 0;
			int ret = 0;
			/* Receive the server response */
			if ((ret = read_msg_from_packet(client_fd, &response_len, &response)) == 0) {
				/* Print what the client received back from the server */
				for (int i = 0; i < response_len; i++) {
					fprintf(stdout, "%c", response[i]);
				}
				free(response);
			}
		/* O flag */
		} else if (arguments.output_flag_set == 1) {
			/* Put all the content of the message into an unpadded chunk of memory */
			char msg[sizeof(request_type)];
			/* First the request type */
			memcpy(&msg[0], &request_type, sizeof(request_type));

			/* Send request to server */
			ssize_t nbytes = write(client_fd, &msg[0], sizeof(msg));
			if ((long long) nbytes < (long long) sizeof(msg)) {
				fprintf(stderr, "ERROR: Failed to send all the data to server\n");
				exit(-1);
			}

			char *response = NULL;
			ssize_t response_len = 0;
			int ret = 0;
			/* Receive the server response */
			if ((ret = read_msg_from_packet(client_fd, &response_len, &response)) == 0) {
				/* Print what the client received back from the server */
				for (int i = 0; i < response_len; i++) {
					fprintf(stdout, "%c", response[i]);
				}
				free(response);
			}
		}
	}

	return 0;
}

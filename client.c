#include <arpa/inet.h> // for inet_aton()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <argp.h>
/* Local Includes */
#include "clientserverutil.h"


/* Program documentation. */
static char doc[] =
	"G2ME-client -- a client for G2ME";

/* A description of the arguments we accept. */
static char args_doc[] = "ARG1 ARG2";

/* Make sure you specify the 3rd field in each entry, otherwise argp will treat the
 * option as not taking an argument */
static struct argp_option options[] = {
	{"address",        'a',  "SERVER_ADDRESS",  0,                    "The ip address of the server" },
	{"matchup-csv",    'C',  "",                OPTION_ARG_OPTIONAL,  "Output a CSV of all the head-to-heads of all the players in the system" }, /* This does not take an argument */
	{"history",        'h',  "PLAYER_NAME",     0,                    "Output a given player's outcome history"},
	{"matchup-table",  'M',  "",                OPTION_ARG_OPTIONAL,  "Output a table of all the head-to-heads of all the players in the system" }, /* This does not take an argument */
	{"output",         'O',  "",                OPTION_ARG_OPTIONAL,  "Output a ranking of all players in the system" }, /* This does not take an argument */
	{"port",           'p',  "PORT",            0,                    "The port of the server" },
	{"record",         'R',  "PLAYER_NAME",     0,                    "Output to FILE instead of standard output" },
	{ 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments {
	/* -a */
	int server_address_flag_set;
	char * server_address;
	/* -C */
	int matchup_csv_flag_set;
	/* -h */
	int history_flag_set;
	/* -M */
	int matchup_table_flag_set;
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
		case 'C':
			arguments->matchup_csv_flag_set = 1;
			break;
		case 'h':
			arguments->player_name = arg;
			arguments->history_flag_set = 1;
			break;
		case 'M':
			arguments->matchup_table_flag_set = 1;
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
	arguments.matchup_csv_flag_set = 0;
	arguments.history_flag_set = 0;
	arguments.matchup_table_flag_set = 0;
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
		if (arguments.matchup_csv_flag_set == 1) {
			request_type = (int) 'C';
		} else if (arguments.history_flag_set == 1) {
			request_type = (int) 'h';
		} else if (arguments.matchup_table_flag_set == 1) {
			request_type = (int) 'M';
		} else if (arguments.output_flag_set == 1) {
			request_type = (int) 'O';
		} else if (arguments.record_flag_set == 1) {
			request_type = (int) 'R';
		}

		/* Convert 'request_type' to network byte order */
		int request_type_n;
		htonarb((char *) &request_type, sizeof(int), (char *) &request_type_n);

		/* h flag or R flag */
		if (arguments.history_flag_set == 1 || arguments.record_flag_set == 1) {
			unsigned int player_name_len = strlen(arguments.player_name);
			unsigned int player_name_len_n;
			htonarb((char *) &player_name_len, sizeof(int), (char *) &player_name_len_n);

			/* Put all the content of the message into an unpadded chunk of memory */
			char msg[sizeof(request_type_n) + sizeof(unsigned int) + strlen(arguments.player_name)];
			/* First the request type */
			memcpy(&msg[0], &request_type_n, sizeof(request_type_n));
			/* then the length of the player name */
			memcpy(&msg[sizeof(request_type_n)], &player_name_len_n, sizeof(player_name_len_n));
			/* then the player name */
			memcpy(&msg[sizeof(request_type_n) + sizeof(unsigned int)], arguments.player_name, strlen(arguments.player_name));

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
		/* C flag, M flag, O flag */
		} else if (arguments.matchup_csv_flag_set == 1 \
			|| arguments.matchup_table_flag_set == 1 \
			|| arguments.output_flag_set == 1) {

			/* Put all the content of the message into an unpadded chunk of memory */
			char msg[sizeof(request_type_n)];
			/* First the request type */
			memcpy(&msg[0], &request_type_n, sizeof(request_type_n));

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

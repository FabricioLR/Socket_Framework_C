#include "../server.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_CONNECTIONS 10
#define PORT 8000

void index_callback(Request *request, Response *response){
	send_response(response, "Content-Type: text/html; charset=utf-8\r\nConnection: Keep-Alive", "Server is running", OK);
}

void test_callback(Request *request, Response *response){
	printf("request body: (%s) %d\n", request->raw_body, request->raw_body_length);

	send_response(response, "", "", OK);
}

void home_callback(Request *request, Response *response){
	FILE *file = fopen("index.html", "r");

	char buffer[2048];
	fread(&buffer, sizeof(char), sizeof(buffer), file);

	send_response(response, "Content-Type: text/html; charset=utf-8", buffer, OK);
}

int main(int argc, char **argv){
	Server *server = init(PORT, MAX_CONNECTIONS);

	add_route(server, "/", GET, index_callback);
	add_route(server, "/home", GET, home_callback);
	add_route(server, "/add-user", POST, test_callback);

	while (1){
		handle_request(server);
	}

	close(server->server_fd);

	return 0;
}

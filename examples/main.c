#include "../server.h"
#include "../json.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_CONNECTIONS 1
#define PORT 8000

void index_callback(Request *request, Response *response){
	printf("(\n%s\n)\n", request->raw_headers);

	Headers *request_headers = headers_parse(request->raw_headers);
 
	printf("%s\n", get_header_value(request_headers, "User-Agent"));

	Headers *response_headers = headers_init();
	add_header(response_headers, "Content-Type", "text/html; charset=utf-8");
	add_header(response_headers, "Hello", "World");

	send_response(response, response_headers, "Server is running", OK);

	free(request);
	free(response);
}

void add_user_callback(Request *request, Response *response){
	printf("request body: (%s) %d\n", request->raw_body, request->raw_body_length);

	JSON *body = json_parse(request->raw_body);

	print_json(body);

	Headers *response_headers = headers_init();
	add_header(response_headers, "Content-Type", "application/json");

	send_response(response, response_headers, "{\"success\": \"true\"}\r\n", OK);

	free(request);
	free(response);
}

int main(int argc, char **argv){
	Server *server = init(PORT, MAX_CONNECTIONS);

	add_route(server, "/", GET, index_callback);
	add_route(server, "/add-user", POST, add_user_callback);

	add_static_directory(server, "public");

	while (1){
		handle_request(server);
	}

	close(server->server_fd);

	return 0;
}

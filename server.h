#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>

#define MAX_ROUTES 10

enum Method {
	GET = 0,
	POST = 1,
	UNDEFINED = 10
};
typedef enum Method Method;

enum Status {
	NOT_FOUND = 404,
	OK = 200,
	METHOD_NOT_ALLOWED = 405
};
typedef enum Status Status;

struct Request {
	int client_fd;

	char *raw_request;
	int raw_request_length;

	char *raw_headers;
	int raw_headers_length;

	char *raw_body;
	int raw_body_length;

	char *path;
	char *protocol;
	Method method;

};
typedef struct Request Request;

struct Response {
	int client_fd;
	
	char *raw_response;
	int raw_response_length;

	/*char *raw_body;
	int raw_body_length;

	char *raw_headers;
	int raw_headers_length;

	Status status_code;*/
	char *protocol;
};
typedef struct Response Response;

struct Route {
	char *path;
	Method method;
	void (*callback)(Request *, Response *);
};
typedef struct Route Route;

struct Server {
	int server_fd;
	Route *routes;
	int max_routes;
	int count_routes;
};
typedef struct Server Server;

Server *init(int port, int max_connections){
	int server_fd, bind_result, listen_result;
	struct sockaddr_in address;

	Server *server = (Server *)malloc(sizeof(Server));
	assert(server != NULL);

	server->routes = (Route *)malloc(MAX_ROUTES * sizeof(Route));
	assert(server->routes != NULL);

	server->max_routes = MAX_ROUTES;
	server->count_routes = 0;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(server_fd >= 0);

	server->server_fd = server_fd;

	assert(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) >= 0);

	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind_result = bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    assert(bind_result >= 0);

    listen_result = listen(server_fd, max_connections);
    assert(listen_result >= 0);

    return server;
}

void add_route(Server *server, char *path, Method method, void *callback){
	assert(server->count_routes < server->max_routes);
	assert(path[0] == '/');
	assert(callback != 0x0);

	int path_length = strlen(path);

	Route *route = &server->routes[server->count_routes];

	route->path = (char *)malloc(path_length);
	assert(route->path != NULL);
	strcpy(route->path, path);
	route->method = method;
	route->callback = callback;

	server->count_routes++;
}

char *substring(char *string, int start, int end){
	char *buffer = (char *)malloc(end - start);
	assert(buffer != NULL);

	int j = 0;
	for (int i = start; i < end; i++){
		buffer[j] = string[i];
		j++;
	}
	buffer[j] = '\0';

	return buffer;
}

void extract_request_line(Request *request){
	int method_end_offset = 0, path_end_offset = 0, protocol_end_offset = 0;

	int space_count = 0;
	for (int i = 0; i < request->raw_request_length; i++){
		if (request->raw_request[i] == ' ' || request->raw_request[i] == '\r' || request->raw_request[i] == '\n') { space_count++; continue; };
		if (space_count > 2) break;

		if (space_count == 0){
			method_end_offset = i;
		} else if (space_count == 1){
			path_end_offset = i;
		} else if (space_count == 2){
			protocol_end_offset = i;
		}
	}

	char *method = substring(request->raw_request, 0, method_end_offset + 1);
	char *path = substring(request->raw_request, method_end_offset + 2, path_end_offset + 1);
	char *protocol = substring(request->raw_request, path_end_offset + 2, protocol_end_offset + 1);

	//printf("(%s) (%s) (%s)\n", method, path, protocol);

	if (strcmp(method, "GET") == 0){
		request->method = GET;
	} else if (strcmp(method, "POST") == 0){
		request->method = POST;
	} else {
		request->method = UNDEFINED;
	}

	free(method);

	request->path = path;
	request->protocol = protocol;

}

void extract_body(Request *request){
	int body_start_offset = 0;
	for (int i = 0; i < request->raw_request_length; i++){
		if (request->raw_request[i] == '\r' && request->raw_request[i + 1] == '\n' && request->raw_request[i + 2] == '\r' && request->raw_request[i + 3] == '\n') { 
			body_start_offset = i + 4;
			break;
		};
	}

	char *body = substring(request->raw_request, body_start_offset, request->raw_request_length);
	int body_length = request->raw_request_length - body_start_offset;

	//printf("(%s)\n", body);

	request->raw_body_length = body_length;
	request->raw_body = (char *)malloc(body_length);
	assert(request->raw_body != NULL);
	strcpy(request->raw_body, body);
}

void parse_raw_request(Request *request){
	extract_request_line(request);
	extract_body(request);
	//extract_headers(request);
}

/*void create_response(Response *response, Status status_code, char *headers, char *body){
	int body_length = strlen(body);
	int headers_length = strlen(headers);

	response->status_code = status_code;
	response->body_length = body_length;
	response->headers_length = headers_length;

	response->body = (char *)malloc(body_length);
	assert(response->body != NULL);
	strcpy(response->body, body);

	response->headers = (char *)malloc(headers_length);
	assert(response->headers != NULL);
	strcpy(response->headers, headers);
}*/

char *status_code_to_text(Status status_code){
	switch (status_code){
		case 404:
			return "Not Found";
		case 200:
			return "Ok";
		case 405:
			return "Method Not Allowed";
		default:
			return "Unkown";
	}
}

void send_response(Response *response, char *headers, char *body, Status status_code){
	char *status_text = status_code_to_text(status_code);

	int body_length = strlen(body);
	int headers_length = strlen(headers);

	response->raw_response = (char *)malloc(1024);
	assert(response->raw_response != NULL);

	int total_length = 0;

	total_length += sprintf(response->raw_response, "%s %d %s\n", response->protocol, status_code, status_text);

	if (headers_length > 0){
		total_length += sprintf(response->raw_response + total_length, "%s\r\n", headers);
	}

	if (body_length > 0){
		total_length += sprintf(response->raw_response + total_length, "Content-Length: %d\r\n\r\n%s", body_length, body);
	} else {
		total_length += sprintf(response->raw_response + total_length, "\r\n\r\n");
	}

	response->raw_response_length = total_length;

	send(response->client_fd, response->raw_response, response->raw_response_length, 0);

	close(response->client_fd);
}

void default_callback(Request *request, Response *response, char *headers, char *message, int status_code){
	//create_response(response, status_code, headers, message);

	send_response(response, headers, message, status_code);
}

void handle_request(Server *server){
	int client_fd = accept(server->server_fd, (struct sockaddr*)NULL, (socklen_t *)NULL);
	assert(client_fd >= 0);
  
	char buffer[10240];
    int length = read(client_fd, buffer, sizeof(buffer) - 1); 
    
    Response *response = (Response *)malloc(sizeof(Response));
    assert(response != NULL);
    Request *request = (Request *)malloc(sizeof(Request));
    assert(request != NULL);

    request->client_fd = client_fd;
    request->raw_request_length = length;
    request->raw_request = (char *)malloc(length);
    assert(request->raw_request != NULL);
    strncpy(request->raw_request, buffer, length);

    parse_raw_request(request);

    response->client_fd = client_fd;
    response->protocol = request->protocol;

    int count = 0;
    for (int i = 0; i < server->count_routes; i++){
    	Route *route = &server->routes[i];

    	if (strcmp(route->path, request->path) == 0){
    		if (route->method == request->method){
    			route->callback(request, response);
    			return;
    		} else {
    			default_callback(request, response, "", "Method not allowed", METHOD_NOT_ALLOWED);
    			return;
    		}
    	}
    }

    default_callback(request, response, "", "Path not found", NOT_FOUND);
    return;
}


#endif // SERVER_H_
#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>

#include "utils.h"

#define MAX_ROUTES 10
#define MAX_DIRECTORIES 10

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

	char *url;
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

struct Directory {
	char *path;
};
typedef struct Directory Directory;

struct Server {
	int server_fd;

	Route *routes;
	int max_routes;
	int count_routes;

	Directory *directories;
	int max_directories;
	int count_directories;
};
typedef struct Server Server;

struct Header {
	char *key;
	char *value;
};
typedef struct Header Header;
struct Headers {
	Header *header;
	int count;
	int capacity;
};
typedef struct Headers Headers;

void add_header(Headers *headers, char *key, char *value){
	if (headers->count >= headers->capacity){
		headers->header = (Header *)realloc(headers->header, sizeof(Headers) * headers->capacity * 2);
		headers->capacity *= 2;
	}
	Header *header = &(headers->header[headers->count]);
	header->key = key;
	header->value = value;
	headers->count++;
}

Headers *headers_parse(char *string){
	int length = strlen(string);

	Headers *headers = (Headers *)malloc(sizeof(Headers));
	assert(headers != NULL);

	headers->count = 0;
	headers->capacity = 10;
	headers->header = (Header *)malloc(sizeof(Header) * headers->capacity);
	assert(headers->header != NULL);

	int value_end = 0, key_end = 0, is_value = 0;
	char *key = NULL;
	for (int i = 0; i < length - 1; i++){
		if (string[i] == ':' && is_value == 0){
			key_end = i;
			key = substring(string, value_end, key_end);
			is_value = 1;
			key_end++;
			continue;
		}
		if (string[i] == '\r' && string[i + 1] == '\n'){
			value_end = i;
			if (string[key_end] == ' ') key_end++;
			char *value = substring(string, key_end, value_end);
			is_value = 0;

			add_header(headers, key, value);
			value_end += 2;
			continue;
		}
	}

	return headers;
}

Headers *headers_init(){
	Headers *headers = (Headers *)malloc(sizeof(Headers));
	assert(headers != NULL);

	headers->count = 0;
	headers->capacity = 10;
	headers->header = (Header *)malloc(sizeof(Header) * headers->capacity);
	assert(headers->header != NULL);

	return headers;
}

char *get_header_value(Headers *headers, char *key){
	assert(headers != NULL);

	for (int i = 0; i < headers->count; i++){
		if (strcmp(headers->header[i].key, key) == 0){
			return headers->header[i].value;
		}
	}

	return "";
}

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

Server *init(int port, int max_connections){
	int server_fd, bind_result, listen_result;
	struct sockaddr_in address;

	Server *server = (Server *)malloc(sizeof(Server));
	assert(server != NULL);

	server->routes = (Route *)malloc(MAX_ROUTES * sizeof(Route));
	assert(server->routes != NULL);

	server->max_routes = MAX_ROUTES;
	server->count_routes = 0;

	server->directories = (Directory *)malloc(MAX_DIRECTORIES * sizeof(Directory));
	assert(server->directories != NULL);

	server->max_directories = MAX_DIRECTORIES;
	server->count_directories = 0;

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

void add_static_directory(Server *server, char *path){
	assert(server->count_directories < server->max_directories);

	int path_length = strlen(path);

	Directory *directory = &server->directories[server->count_directories];

	directory->path = path;

	server->count_directories++;
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

void extract_request_line(Request *request){
	int method_end_offset = 0, url_end_offset = 0, protocol_end_offset = 0;

	int space_count = 0;
	for (int i = 0; i < request->raw_request_length; i++){
		if (request->raw_request[i] == ' ' || request->raw_request[i] == '\r' || request->raw_request[i] == '\n') { space_count++; continue; };
		if (space_count > 2) break;

		if (space_count == 0){
			method_end_offset = i;
		} else if (space_count == 1){
			url_end_offset = i;
		} else if (space_count == 2){
			protocol_end_offset = i;
		}
	}

	char *method = substring(request->raw_request, 0, method_end_offset + 1);
	char *url = substring(request->raw_request, method_end_offset + 2, url_end_offset + 1);
	char *protocol = substring(request->raw_request, url_end_offset + 2, protocol_end_offset + 1);

	//printf("(%s) (%s) (%s)\n", method, path, protocol);

	if (strcmp(method, "GET") == 0){
		request->method = GET;
	} else if (strcmp(method, "POST") == 0){
		request->method = POST;
	} else {
		request->method = UNDEFINED;
	}

	free(method);

	request->url = url;
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

	request->raw_body = body;
	request->raw_body_length = body_length;
}

void extract_headers(Request *request){
	int request_line_end_offset = 0, body_start_offset = 0;
	for (int i = 0; i < request->raw_request_length; i++){
		if (request_line_end_offset == 0){
			if (request->raw_request[i] == '\r' && request->raw_request[i + 1] == '\n') request_line_end_offset = i + 2;
		}
		if (body_start_offset == 0){
			if (request->raw_request[i] == '\r' && request->raw_request[i + 1] == '\n' && request->raw_request[i + 2] == '\r' && request->raw_request[i + 3] == '\n') { 
				body_start_offset = i;
				break;
			};
		}
	}

	char *headers = substring(request->raw_request, request_line_end_offset, body_start_offset);
	int headers_length = body_start_offset - request_line_end_offset;

	request->raw_headers = headers;
	request->raw_request_length = headers_length;
}

void parse_raw_request(Request *request){
	extract_request_line(request);
	extract_body(request);
	extract_headers(request);
}

void send_response(Response *response, Headers *headers, char *body, Status status_code){
	char *status_text = status_code_to_text(status_code);

	int body_length = strlen(body);
	//int headers_length = strlen(headers);

	response->raw_response = (char *)malloc(1024);
	assert(response->raw_response != NULL);

	int total_length = 0;

	total_length += sprintf(response->raw_response, "%s %d %s\n", response->protocol, status_code, status_text);

	if (headers != NULL){
		for (int i = 0; i < headers->count; i++){
			total_length += sprintf(response->raw_response + total_length, "%s: %s\r\n", headers->header[i].key, headers->header[i].value);
		}
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

char *url_get_path(char *url){
	int length = strlen(url);

	int path_end_offset = length;
	for (int i = 0; i < length; i++){
		if (url[i] == '?' || url[i] == '#'){
			path_end_offset = i;
			break;			
		}
	}
	return substring(url, 0, path_end_offset);
}

char *url_get_query_string(char *url){
	int length = strlen(url);

	int query_string_start_offset = 0, query_string_end_offset = length;
	for (int i = 0; i < length; i++){
		if (url[i] == '?'){
			query_string_start_offset = i + 1;			
		} else if (url[i] == '#'){
			query_string_end_offset = i;
		}
	}
	return substring(url, query_string_start_offset, query_string_end_offset);
}

char *url_get_fragment(char *url){
	int length = strlen(url);

	int fragment_start_offset = length;
	for (int i = 0; i < length; i++){
		if (url[i] == '#'){
			fragment_start_offset = i + 1;			
			break;
		}
	}
	return substring(url, fragment_start_offset, length);
}

void default_callback(Request *request, Response *response, Headers *headers, char *message, int status_code){
	Headers *response_headers = headers_init();
	add_header(response_headers, "Content-Type", "text/html; charset=utf-8");

	send_response(response, response_headers, message, status_code);
}

void default_static_directory_callback(Request *request, Response *response, char *file_path){
	FILE *file = fopen(file_path, "r");

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = malloc(size + 10);
	assert(buffer != NULL);
	fread(buffer, sizeof(char), size, file);
	buffer[size] = '\0';

	Headers *response_headers = headers_init();
	add_header(response_headers, "Content-Type", "text/html; charset=utf-8");

	send_response(response, response_headers, buffer, OK);

	free(request);
	free(response);
	fclose(file);
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

    char *path = url_get_path(request->url);

    for (int i = 0; i < server->count_directories; i++){
    	Directory *directory = &server->directories[i]; 

    	int directory_path_length = strlen(directory->path);
    	int path_length = strlen(path);

    	char *full_path = malloc(directory_path_length + path_length + 5);
    	assert(full_path != NULL);

    	sprintf(full_path, "%s%s", directory->path, path);

    	if (access(full_path, F_OK) == 0) {
    		default_static_directory_callback(request, response, full_path);
    		return;
    	}
    }

    for (int i = 0; i < server->count_routes; i++){
    	Route *route = &server->routes[i];

    	if (strcmp(route->path, path) == 0){
    		if (route->method == request->method){
    			route->callback(request, response);
    			return;
    		} else {
    			default_callback(request, response, NULL, "Method not allowed", METHOD_NOT_ALLOWED);
    			return;
    		}
    	}
    }

    default_callback(request, response, NULL, "Path not found", NOT_FOUND);
    return;
}


#endif // SERVER_H_
# Socket Framework

![GitHub repo size](https://img.shields.io/github/repo-size/FabricioLR/Socket_Framework_C?style=for-the-badge)
![GitHub language count](https://img.shields.io/github/languages/count/FabricioLR/Socket_Framework_C?style=for-the-badge)
![GitHub forks](https://img.shields.io/github/forks/FabricioLR/Socket_Framework_C?style=for-the-badge)
![Bitbucket open issues](https://img.shields.io/bitbucket/issues/FabricioLR/Socket_Framework_C?style=for-the-badge)
![Bitbucket open pull requests](https://img.shields.io/bitbucket/pr-raw/FabricioLR/Socket_Framework_C?style=for-the-badge)

> Implementação de um string builder utilizando linkedlist escrito em C.

## 🚀 Instalando

Apenas copie o arquivo ![server.h](https://github.com/FabricioLR/Socket_Framework_C/blob/master/server.h) para dentro do seu projeto

## ☕ Usando

```
void index_callback(Request *request, Response *response){
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
```

Saída esperada

```
$cd examples && make exec &
$curl http://localhost:8000/index.html
<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<title>Example</title>
</head>
<body>
	<div>
		Hello, world!
	</div>
</body>
</html>
```


## 📫 Contribuindo

Para contribuir, siga estas etapas:

1. Fork este repositório.
2. Crie um branch: `git checkout -b <nome_branch>`.
3. Faça suas alterações e confirme-as: `git commit -m '<mensagem_commit>'`
4. Envie para o branch original: `git push origin <nome_do_projeto> / <local>`
5. Crie um Pull Request.

Como alternativa, consulte a documentação do GitHub em [como criar uma solicitação pull](https://help.github.com/en/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request).

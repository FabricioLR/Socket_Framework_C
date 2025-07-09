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
#include "server.h"

void home_callback(Request *request, Response *response){
	FILE *file = fopen("index.html", "r");

	char buffer[2048];
	fread(&buffer, sizeof(char), sizeof(buffer), file);

	send_response(response, "Content-Type: text/html; charset=utf-8", buffer, OK);
}

int main(int argc, char **argv){
	Server *server = init(8000, 10);

	add_route(server, "/home", GET, home_callback);

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
$curl http://localhost:8000/home
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

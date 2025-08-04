async function fn(){
	const response = await fetch("http://localhost:8000/", {
		method: "GET"
	});
	/*const response = await fetch("http://localhost:8000/add-user", {
		method: "POST",
		headers: {
			"Content-Type": "application/json"
		},
		body: JSON.stringify({username: "fabricio", password: 123456})
	})*/

	console.log(response.status, await response.text());
}


fn()
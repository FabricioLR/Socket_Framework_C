import express from "express";

const app = express();

app.use(express.json());
app.use(express.urlEncoded({ extended: false }));

app.get("/", (request, response) => {
	return response.status(200).send("Server is running");
});

app.post("/test", (request, response) => {
	console.log(request.body);

	return response.status(200).send();
});

app.listen(8000, () => { console.log("Server is running on PORT 8000") });
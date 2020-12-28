all:client server

client:
	g++ client.cc -o client -lpthread

server:
	g++ server.cc -o server -lpthread

clean:
	rm -rf client server

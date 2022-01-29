all:
	gcc -g server.c -o server
	gcc -g client.c -o client
server:
	gcc -g server.c -o server
client:	
	gcc -g client.c -o client
clean:
	rm server client

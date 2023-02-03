all:chat-client chat-server
chat-client:chat-client.c
	gcc chat-client.c -o chat-client
chat-server:chat-server.c
	gcc chat-server.c -o chat-server

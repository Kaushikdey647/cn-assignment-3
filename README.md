# cn-assignment-3

## 1. Introduction

This is the third assignment for the course of Computer Networks...

## 2. Problem Statement

**Implementation of a chat server using TCP socket.**

- The server will ensure smooth connection between a pair of clients.
- A client can talk to another client during a session if the requested client is free.
- The list of connected clients (with the server) with status (BUSY, FREE, etc.) should be shown to a requesting client.
- The client sends a match request to the server and depending on the availability of therequested client, a separate communication channel may be established between the clients.
- This can be implemented in two ways:
	- all messages are exchanged via the server
	- or, a separate connection is established between the clients.
- The chat session can be ended from any side of both clients by sending a goodbye message.
- The other side is bound to close the chat session upon receiving the goodbye message.
- But, both the clients remain active in the server’s list and can start chatting on a different session.
- Only close command from a client terminates a connection between the client and the server.
- The server runs forever.
- The above features are mandatory and you can include any other ornamental features as per your choice. But, the grading will be done only based on the above features.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5000
#define BUFFER_SIZE 1024

struct sockaddr_in bootstrap_socket_address_tcp(char* ip_addr, int port){
    
    //Used to store the address of the server for IPv4
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if( strlen(ip_addr) == 0 ){
        server_addr.sin_addr.s_addr = INADDR_ANY;
    }
    else{
        inet_pton(AF_INET, ip_addr, &server_addr.sin_addr);
    }

    return server_addr;
}

int main() { 
    

    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    // create socket

    int sock_fd = socket(
        AF_INET, //Address Family: IPv4
        SOCK_STREAM, //Type: TCP
        0 // IP Layer underneath the TCP layer
    );

    if ( sock_fd < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    // set server address and port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // convert IP address from string to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        return EXIT_FAILURE;
    }

    // connect to server
    int conn = connect(
        sock,
        (struct sockaddr *)&serv_addr,
        sizeof(serv_addr)) < 0) {
        perror("connect failed");
        return EXIT_FAILURE;
    }

    printf("Connected to server\n");

    // send messages to server
    while (1) {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("send failed");
            break;
        }
    }

    // close socket
    close(sock);

    return EXIT_SUCCESS;
}
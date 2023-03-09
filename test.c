#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0
#define PORT 8888

#define MAX_CLI 32
#define BUFF_SIZE 2048

int main(int argc, char *argv[])
{
    // Create a socket
    int sock_fd = socket(
        AF_INET, //Address Family: IPv4
        SOCK_STREAM, //Type: TCP
        0 // IP Layer underneath the TCP layer
    );

    //Used to store the address of the server for IPv4
    struct sockaddr_in server_addr;

    char* server_ip = "142.251.214.142";

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);

    //Converts the IP address from text to binary form
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    int conn = connect(
        sock_fd, //socket file descriptor
        (struct sockaddr *)&server_addr, //address of the server
        sizeof(server_addr) //size of the address
    );

    if(conn){
        printf("Connection failed\n");
        return 1;
    }
    else{
        printf("Connection successful\n");
    }

    char* msg;

    msg = "GET \\ HTTP/1.1\r\nHost:google.com\r\n\r";

    send(
        sock_fd, //socket file descriptor
        msg, //buffer to send
        strlen(msg), //length of the buffer
        0 //flags
    );

    char buffer[65536];

    recv(
        sock_fd, //socket file descriptor
        buffer, //buffer to store the data
        65536, //length of the buffer
        0 //flags
    );

    printf("%s", buffer);

    return 0;
}

// 127.0.0.1
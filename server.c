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

typedef struct{
    struct sockaddr_in address;
    int socket_fd;
    char name[32];
    char status[4];
} cli_t;

cli_t *clients[MAX_CLI];

pthread_t thread_id[MAX_CLI];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex for clients

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

void* handle_client(void* arg){

    //get the client file descriptor
    int cli_fd = *(int*)arg;

    //create a buffer to store the message
    char buffer[BUFF_SIZE];

    printf("Client connected\n");

    ssize_t read_size;

    char username[32];

    //write request for username into buffer
    sprintf(buffer, "Enter your username: ");

    //send the request

    send(cli_fd, buffer, strlen(buffer), 0);
    
    //receive username

    read_size = recv(cli_fd, username, 32, 0);

    //write terminating character
    username[read_size] = '\0';

    //receive messages in loop
    while( (read_size = recv(cli_fd, buffer, BUFF_SIZE, 0)) > 0 ){

        //write terminating character
        buffer[read_size] = '\0';

        //print the message

        printf("[%s]: %s", username, buffer);

        //clear the buffer
        memset(buffer, 0, BUFF_SIZE);
    }

    close(cli_fd);

    printf("Client disconnected\n");

    return NULL;

}

int main( int argc, char *argv[] )
{
    int cli_fd; //client file descriptor

    int cli_len; //client length

    int cli_ptr = 0; //client pointer

    // Create a socket
    int sock_fd = socket(
        AF_INET, //Address Family: IPv4
        SOCK_STREAM, //Type: TCP
        0 // IP Layer underneath the TCP layer
    );

    //create server socket object
    struct sockaddr_in server_addr = bootstrap_socket_address_tcp("", PORT);

    //bind the socket to the address and port
    int bind_status = bind(
        sock_fd, //socket file descriptor
        (struct sockaddr *)&server_addr, //address of the server
        sizeof(server_addr) //size of the address
    );

    if(bind_status){
        printf("Bind failed\n");
        return 1;
    }

    //listen for connections

    int listen_status = listen(
        sock_fd, //socket file descriptor
        MAX_CLI //number of connections to queue
    );

    if(listen_status){
        printf("Listen failed\n");
        return 1;
    }

    while(1){
        
        // accept connection from an incoming client
        cli_fd = accept(
            sock_fd, //socket file descriptor
            (struct sockaddr *)&server_addr, //address of the server
            (socklen_t*)&cli_len //size of the address
        );

        if(cli_fd < 0){
            printf("Accept failed\n");
            return 1;
        }

        //create a thread to handle the client
        pthread_create(
            &thread_id[cli_ptr], //thread id
            NULL, //thread attributes
            handle_client, //function to run
            (void*) &cli_fd //arguments to the function
        );
        
        //increment the client pointer
        cli_ptr++;

        //if the client pointer is greater than the max number of clients
        if(cli_ptr == MAX_CLI){
            printf("Max clients reached\n");
            return 1;
        }
    }

    close(sock_fd);

    return 0;

}
#include "lib/microtcp.h"
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_port = atoi(argv[1]);

    // Create and bind the server socket
    microtcp_sock_t server_socket = microtcp_socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (microtcp_bind(&server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Failed to bind the server socket\n");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        // Accept a connection
        microtcp_sock_t client_socket = server_socket;  // Copy the server socket
        while(1){
            if (microtcp_accept(&client_socket, (struct sockaddr *)&client_address, &client_address_len) < 0) {
                fprintf(stderr, "Failed to accept a connection\n");
                return -1;
            }
            client_socket.address = client_address;
            client_socket.address_len = client_address_len;
            break;
        }
        printf("RECEIVED PACKET FROM THE CLIENT\n");
        // At this point, the connection is established, and you can start sending and receiving data with the client
        
        // Close the microTCP socket for the accepted connection
        //close(client_socket.sd);
        
        while(1){
        // Close the microTCP socket for the accepted connection
        if (microtcp_shutdown(&client_socket , 1) < 0) {
            fprintf(stderr, "Failed to Shutdown a Connection \n");
            return -1;
        }
        break;    
        }
        return 0;
}
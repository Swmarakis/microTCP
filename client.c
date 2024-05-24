#include "lib/microtcp.h"
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Create a microTCP socket
    microtcp_sock_t client_socket = microtcp_socket(AF_INET, SOCK_DGRAM, 0);

    // Set up the server address
    struct sockaddr_in server_address;
    socklen_t address_len = sizeof(server_address);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_address.sin_addr);

    printf("Before microtcp_connect - server_address.sin_port: %u\n", ntohs(server_address.sin_port));

    // Print the values for verification
    printf("Server IP: %s\n", server_ip);
    printf("Server Port: %d\n", server_port);
    printf("Socket Descriptor: %d\n", client_socket.sd);

    // Connect to the server
    if (microtcp_connect(&client_socket, (struct sockaddr *)&server_address, address_len) < 0) {
        fprintf(stderr, "Failed to connect to the server\n");
        printf("After microtcp_connect - server_address.sin_port: %u\n", ntohs(server_address.sin_port));
        exit(EXIT_FAILURE);
    } else {
        printf("Successfully connected to the server\n");
    }

    client_socket.address = server_address;
    client_socket.address_len = address_len;
    //close(client_socket.sd);

    // Close the microTCP socket
    if (microtcp_shutdown(&client_socket, 0  ) < 0) {
        fprintf(stderr, "Failed to Shutdown a Connection \n");
        return -1;
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>

int hostname_to_ip(char* hostname, char* ip_dest){
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;  // Use IPv4
    hints.ai_socktype = SOCK_STREAM;  // Use TCP

    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    for (struct addrinfo* addr = res; addr != NULL; addr = addr->ai_next) {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*) addr->ai_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
        printf("IP Address: %s\n", ip);
        strcpy(ip_dest, ip);
        return 0; 
    }
}

int main(int argc, char** argv) {
    int client_socket;
    struct sockaddr_in server_address;

    char SERVER_IP[20];
    hostname_to_ip(argv[1], SERVER_IP);
    int SERVER_PORT = atoi(argv[2]);

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        fprintf(stderr, "Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server 
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to %s:%d\n", SERVER_IP, SERVER_PORT);

    char buffer[1024];
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_socket, &read_fds);

        // Set a timeout to avoid blocking indefinitely
        struct timeval timeout;
        timeout.tv_sec = 1;  // 1 second
        timeout.tv_usec = 0;

        // Use select to check for available data to read
        int ready = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);
        if (ready < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(client_socket, &read_fds)) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t n = read(client_socket, buffer, sizeof(buffer) - 1);
            if (n <= 0) {
                printf("Connection closed by the server.\n");
                break;
            }
            printf("%s", buffer);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            memset(buffer, 0, sizeof(buffer));
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                write(client_socket, buffer, strlen(buffer));
            }
        }
    }

    // Close the socket
    close(client_socket);
    return 0;
}

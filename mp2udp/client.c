#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Send initial message to server to identify the client
    printf("Connecting to the server...\n");
    sendto(sock, "Hello from client\n", strlen("Hello from client\n"), 0, (struct sockaddr*)&server_addr, addr_len);

    // Main loop for chatting
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // Receive message from the server
        recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &addr_len);
        printf("%s", buffer);  // Display message from the other client or prompt

        // Check if it's the client's turn
        if (strstr(buffer, "Your turn!") != NULL) {
            // Send message to server
            printf("You: ");
            fgets(buffer, BUFFER_SIZE, stdin);
            sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addr_len);
        }
    }

    close(sock);
    return 0;
}

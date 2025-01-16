#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address / Address not supported\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }

    // Main loop to handle communication with the server
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // Read message from the server
        valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Server disconnected.\n");
            break;
        }
        buffer[valread] = '\0';
        printf("%s", buffer);

        // If the server is asking whether to start the game or play again
        if (strstr(buffer, "Do you want to play") != NULL) {
            printf("Your response (y/n): ");
            fgets(buffer, BUFFER_SIZE, stdin);
            send(sock, buffer, strlen(buffer), 0);
            continue;
        }

        // If the game ended (win, draw), the game loop will break
        if (strstr(buffer, "Wins!") != NULL || strstr(buffer, "Draw!") != NULL) {
            // Ask if the player wants to play again after game ends
            continue;
        }

        // Prompt for input if it's the user's turn
        if (strstr(buffer, "Your turn!") != NULL) {
            printf("Enter your move (row,col): ");
            fgets(buffer, BUFFER_SIZE, stdin);
            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}

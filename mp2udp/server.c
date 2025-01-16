#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Initialize an empty Tic-Tac-Toe board
void initializeBoard(char board[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = '-';
        }
    }
}

// Print the Tic-Tac-Toe board
void printBoard(char board[3][3]) {
    printf("\nCurrent Board:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

// Check if the game is won
int checkWin(char board[3][3], char symbol) {
    for (int i = 0; i < 3; i++) {
        // Check rows and columns
        if ((board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol) ||
            (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol))
            return 1;
    }
    // Check diagonals
    if ((board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol) ||
        (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol))
        return 1;

    return 0;
}

// Check if the board is full (draw)
int checkDraw(char board[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == '-') {
                return 0;
            }
        }
    }
    return 1;
}

// Convert the board into a string for sending to the clients
char* condenseBoard(char board[3][3]) {
    static char condensed[20];
    int index = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            condensed[index++] = board[i][j];
        }
        condensed[index++] = '\n';
    }
    condensed[index] = '\0';
    return condensed;
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client1_addr, client2_addr;
    socklen_t client1_len = sizeof(client1_addr);
    socklen_t client2_len = sizeof(client2_addr);
    char buffer[BUFFER_SIZE];
    char board[3][3];
    int client1_connected = 0, client2_connected = 0;
    int turn = 1;  // Start with Client 1's turn

    // Initialize the Tic-Tac-Toe board
    initializeBoard(board);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running. Waiting for two clients to join...\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // First, check if both clients have connected
        if (!client1_connected) {
            // Receive message from the first client
            recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client1_addr, &client1_len);
            client1_connected = 1;
            printf("Client 1 connected.\n");
            sendto(server_fd, "You are Player 1 (X). Waiting for Player 2...\n", 46, 0, (struct sockaddr*)&client1_addr, client1_len);
        } else if (!client2_connected) {
            // Receive message from the second client
            recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client2_addr, &client2_len);
            client2_connected = 1;
            printf("Client 2 connected.\n");
            sendto(server_fd, "You are Player 2 (O). Game starting...\n", 39, 0, (struct sockaddr*)&client2_addr, client2_len);
            sendto(server_fd, "Player 2 has joined. Game starting...\n", 38, 0, (struct sockaddr*)&client1_addr, client1_len);

            // Prompt Player 1 to make the first move
            sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client1_addr, client1_len);
            sendto(server_fd, "Your turn! Enter row and column (e.g., 1 1):\n", 45, 0, (struct sockaddr*)&client1_addr, client1_len);
        } else {
            // Turn-based message handling
            if (turn == 1) {
                // Receive move from Player 1
                recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client1_addr, &client1_len);
                int row, col;
                sscanf(buffer, "%d %d", &row, &col);
                row--; col--;  // Convert to 0-indexed

                // Validate move
                if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == '-') {
                    board[row][col] = 'X';
                    printBoard(board);

                    // Check for win or draw
                    if (checkWin(board, 'X')) {
                        sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client1_addr, client1_len);
                        sendto(server_fd, "Player 1 (X) Wins!\n", 19, 0, (struct sockaddr*)&client1_addr, client1_len);
                        sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client2_addr, client2_len);
                        sendto(server_fd, "Player 1 (X) Wins!\n", 19, 0, (struct sockaddr*)&client2_addr, client2_len);
                        break;
                    } else if (checkDraw(board)) {
                        sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client1_addr, client1_len);
                        sendto(server_fd, "It's a Draw!\n", 13, 0, (struct sockaddr*)&client1_addr, client1_len);
                        sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client2_addr, client2_len);
                        sendto(server_fd, "It's a Draw!\n", 13, 0, (struct sockaddr*)&client2_addr, client2_len);
                        break;
                    }

                    // Switch turn to Player 2
                    turn = 2;
                    sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client2_addr, client2_len);
                    sendto(server_fd, "Your turn! Enter row and column (e.g., 1 1):\n", 45, 0, (struct sockaddr*)&client2_addr, client2_len);
                } else {
                    // Invalid move, ask Player 1 to try again (stay on Player 1's turn)
                    sendto(server_fd, "Invalid move, try again.\n", 25, 0, (struct sockaddr*)&client1_addr, client1_len);
                    sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client1_addr, client1_len);
                    sendto(server_fd, "Your turn! Enter row and column (e.g., 1 1):\n", 45, 0, (struct sockaddr*)&client1_addr, client1_len);
                }
            } else if (turn == 2) {
                // Receive move from Player 2
                recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client2_addr, &client2_len);
                int row, col;
                sscanf(buffer, "%d %d", &row, &col);
                row--; col--;  // Convert to 0-indexed

                // Validate move
                if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == '-') {
                    board[row][col] = 'O';
                    printBoard(board);

                    // Check for win or draw
                    if (checkWin(board, 'O')) {
                        sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client1_addr, client1_len);
                        sendto(server_fd, "Player 2 (O) Wins!\n", 19, 0, (struct sockaddr*)&client1_addr, client1_len);
                        sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client2_addr, client2_len);
                        sendto(server_fd, "Player 2 (O) Wins!\n", 19, 0, (struct sockaddr*)&client2_addr, client2_len);
                        break;
                    } else if (checkDraw(board)) {
                        sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client1_addr, client1_len);
                        sendto(server_fd, "It's a Draw!\n", 13, 0, (struct sockaddr*)&client1_addr, client1_len);
                        sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client2_addr, client2_len);
                        sendto(server_fd, "It's a Draw!\n", 13, 0, (struct sockaddr*)&client2_addr, client2_len);
                        break;
                    }

                    // Switch turn to Player 1
                    turn = 1;
                    sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client1_addr, client1_len);
                    sendto(server_fd, "Your turn! Enter row and column (e.g., 1 1):\n", 45, 0, (struct sockaddr*)&client1_addr, client1_len);
                } else {
                    // Invalid move, ask Player 2 to try again (stay on Player 2's turn)
                    sendto(server_fd, "Invalid move, try again.\n", 25, 0, (struct sockaddr*)&client2_addr, client2_len);
                    sendto(server_fd, condenseBoard(board), strlen(condenseBoard(board)), 0, (struct sockaddr*)&client2_addr, client2_len);
                    sendto(server_fd, "Your turn! Enter row and column (e.g., 1 1):\n", 45, 0, (struct sockaddr*)&client2_addr, client2_len);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}

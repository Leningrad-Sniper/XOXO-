#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Initialize the empty Tic-Tac-Toe board
void initializeBoard(char board[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = '-';
        }
    }
}

// Convert the board into a string for transmission
char* condenseBoard(char board[3][3]) {
    char* condensed = (char*)malloc(20 * sizeof(char));  // 9 cells + 3 newline + null terminator
    int index = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            condensed[index++] = board[i][j];
        }
        condensed[index++] = '\n';  // Add newline for each row
    }
    condensed[index] = '\0';
    return condensed;
}

// Check if a player has won the game
int checkWin(char board[3][3], char player) {
    // Check rows and columns
    for (int i = 0; i < 3; i++) {
        if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
            (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
            return 1;
        }
    }
    // Check diagonals
    if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
        (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
        return 1;
    }
    return 0;
}

// Check if the board is full (draw)
int checkDraw(char board[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == '-') {
                return 0; // Board is not full, no draw
            }
        }
    }
    return 1; // Board is full, it's a draw
}

// Reset game state and board
void resetGame(char board[3][3], int* turn, int* game_active) {
    initializeBoard(board);
    *turn = 1;  // Reset to Player 1's turn
    *game_active = 1;  // Activate game again
}

int main() {
    int server_fd, client1_fd = -1, client2_fd = -1, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    char board[3][3];  // Tic-Tac-Toe board
    int turn = 1;  // Start with Player 1's turn ('X')
    int game_active = 0;  // Game starts when both players confirm they are ready
    int player1_ready = 0, player2_ready = 0;
    int player1_continue = 0, player2_continue = 0;
    
    initializeBoard(board);  // Initialize the board at the start

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for two players to join...\n");

    // Main loop
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        if (client1_fd > 0) FD_SET(client1_fd, &readfds);
        if (client2_fd > 0) FD_SET(client2_fd, &readfds);

        int max_sd = server_fd;
        if (client1_fd > max_sd) max_sd = client1_fd;
        if (client2_fd > max_sd) max_sd = client2_fd;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // Handle new client connection
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            if (client1_fd == -1) {
                client1_fd = new_socket;
                printf("Player 1 (X) joined. Waiting for Player 2...\n");
                send(client1_fd, "You are Player 1 (X). Waiting for Player 2...\n", 45, 0);
            } else if (client2_fd == -1) {
                client2_fd = new_socket;
                printf("Player 2 (O) joined. Waiting for both players to confirm...\n");
                send(client1_fd, "Player 2 (O) joined. Do you want to play (y/n)?\n", 47, 0);
                send(client2_fd, "You are Player 2 (O). Do you want to play (y/n)?\n", 47, 0);
            } else {
                printf("Max clients reached. Connection rejected.\n");
                close(new_socket);
            }
        }

        // Check if Player 1 is ready
        if (client1_fd > 0 && FD_ISSET(client1_fd, &readfds) && !player1_ready) {
            int valread = read(client1_fd, buffer, BUFFER_SIZE);
            if (valread == 0) {
                close(client1_fd);
                client1_fd = -1;
                continue;
            }
            buffer[valread] = '\0';
            if (strcmp(buffer, "y\n") == 0) {
                printf("Player 1 is ready to play.\n");
                player1_ready = 1;
            } else if (strcmp(buffer, "n\n") == 0) {
                printf("Player 1 declined to play.\n");
                send(client2_fd, "Player 1 declined. Exiting...\n", 31, 0);
                close(client1_fd);
                close(client2_fd);
                break;
            }
        }

        // Check if Player 2 is ready
        if (client2_fd > 0 && FD_ISSET(client2_fd, &readfds) && !player2_ready) {
            int valread = read(client2_fd, buffer, BUFFER_SIZE);
            if (valread == 0) {
                close(client2_fd);
                client2_fd = -1;
                continue;
            }
            buffer[valread] = '\0';
            if (strcmp(buffer, "y\n") == 0) {
                printf("Player 2 is ready to play.\n");
                player2_ready = 1;
            } else if (strcmp(buffer, "n\n") == 0) {
                printf("Player 2 declined to play.\n");
                send(client1_fd, "Player 2 declined. Exiting...\n", 31, 0);
                close(client1_fd);
                close(client2_fd);
                break;
            }
        }

        // Start the game if both players are ready
        if (player1_ready && player2_ready && !game_active) {
            printf("Both players are ready. Starting the game!\n");
            game_active = 1;

            // Send initial board state
            char* boardState = condenseBoard(board);
            send(client1_fd, boardState, strlen(boardState), 0);
            send(client2_fd, boardState, strlen(boardState), 0);
            free(boardState);

            send(client1_fd, "Your turn! Enter row and column (e.g., 1 1): ", 45, 0);
            send(client2_fd, "Waiting for Player 1 (X) to make a move...\n", 43, 0);
        }

        // Handle moves from Player 1 (X)
        if (client1_fd > 0 && FD_ISSET(client1_fd, &readfds) && turn == 1 && game_active) {
            int valread = read(client1_fd, buffer, BUFFER_SIZE);
            if (valread == 0) {
                close(client1_fd);
                client1_fd = -1;
                continue;
            }
            buffer[valread] = '\0';

            int row, col;
            sscanf(buffer, "%d %d", &row, &col);
            row--; col--;  // Convert to 0-based index

            if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == '-') {
                board[row][col] = 'X';  // Valid move, place 'X'

                if (checkWin(board, 'X')) {
                    game_active = 0;
                    char* boardState = condenseBoard(board);
                    send(client1_fd, boardState, strlen(boardState), 0);
                    send(client2_fd, boardState, strlen(boardState), 0);
                    send(client1_fd, "Player 1 (X) Wins!\n", 19, 0);
                    send(client2_fd, "Player 1 (X) Wins!\n", 19, 0);
                    free(boardState);
                    // Ask both players if they want to play again
                    send(client1_fd, "Do you want to play again (y/n)?\n", 33, 0);
                    send(client2_fd, "Do you want to play again (y/n)?\n", 33, 0);
                    continue;
                }

                if (checkDraw(board)) {
                    game_active = 0;
                    char* boardState = condenseBoard(board);
                    send(client1_fd, boardState, strlen(boardState), 0);
                    send(client2_fd, boardState, strlen(boardState), 0);
                    send(client1_fd, "It's a Draw!\n", 13, 0);
                    send(client2_fd, "It's a Draw!\n", 13, 0);
                    free(boardState);
                    // Ask both players if they want to play again
                    send(client1_fd, "Do you want to play again (y/n)?\n", 33, 0);
                    send(client2_fd, "Do you want to play again (y/n)?\n", 33, 0);
                    continue;
                }

                // Send the updated board to both players
                char* boardState = condenseBoard(board);
                send(client1_fd, boardState, strlen(boardState), 0);
                send(client2_fd, boardState, strlen(boardState), 0);
                free(boardState);

                // Switch to Player 2's turn
                turn = 2;
                send(client1_fd, "Waiting for Player 2 (O) to make a move...\n", 43, 0);
                send(client2_fd, "Your turn! Enter row and column (e.g., 1 1): ", 45, 0);
            } else {
                send(client1_fd, "Invalid move! Try again: ", 25, 0);
            }
        }

        // Handle moves from Player 2 (O)
        if (client2_fd > 0 && FD_ISSET(client2_fd, &readfds) && turn == 2 && game_active) {
            int valread = read(client2_fd, buffer, BUFFER_SIZE);
            if (valread == 0) {
                close(client2_fd);
                client2_fd = -1;
                continue;
            }
            buffer[valread] = '\0';

            int row, col;
            sscanf(buffer, "%d %d", &row, &col);
            row--; col--;  // Convert to 0-based index
        
            if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == '-') {
                board[row][col] = 'O';  // Valid move, place 'O'

                if (checkWin(board, 'O')) {
                    game_active = 0;
                    char* boardState = condenseBoard(board);
                    send(client1_fd, boardState, strlen(boardState), 0);
                    send(client2_fd, boardState, strlen(boardState), 0);
                    send(client1_fd, "Player 2 (O) Wins!\n", 19, 0);
                    send(client2_fd, "Player 2 (O) Wins!\n", 19, 0);
                    free(boardState);
                    // Ask both players if they want to play again
                    send(client1_fd, "Do you want to play again (y/n)?\n", 33, 0);
                    send(client2_fd, "Do you want to play again (y/n)?\n", 33, 0);
                    continue;
                }

                if (checkDraw(board)) {
                    game_active = 0;
                    char* boardState = condenseBoard(board);
                    send(client1_fd, boardState, strlen(boardState), 0);
                    send(client2_fd, boardState, strlen(boardState), 0);
                    send(client1_fd, "It's a Draw!\n", 13, 0);
                    send(client2_fd, "It's a Draw!\n", 13, 0);
                    free(boardState);
                    // Ask both players if they want to play again
                    send(client1_fd, "Do you want to play again (y/n)?\n", 33, 0);
                    send(client2_fd, "Do you want to play again (y/n)?\n", 33, 0);
                    continue;
                }

                // Send the updated board to both players
                char* boardState = condenseBoard(board);
                send(client1_fd, boardState, strlen(boardState), 0);
                send(client2_fd, boardState, strlen(boardState), 0);
                free(boardState);

                // Switch to Player 1's turn
                turn = 1;
                send(client1_fd, "Your turn! Enter row and column (e.g., 1 1): ", 45, 0);
                send(client2_fd, "Waiting for Player 1 (X) to make a move...\n", 43, 0);
            } else {
                send(client2_fd, "Invalid move! Try again: ", 25, 0);
            }
        }

        // Handle post-game responses for playing again
        if (player1_ready && player2_ready && !game_active) {
            if (FD_ISSET(client1_fd, &readfds)) {
                int valread = read(client1_fd, buffer, BUFFER_SIZE);
                buffer[valread] = '\0';
                if (strcmp(buffer, "y\n") == 0) {
                    player1_continue = 1;
                } else {
                    player1_continue = 0;
                    send(client2_fd, "Player 1 does not want to play again. Exiting...\n", 49, 0);
                    close(client1_fd);
                    close(client2_fd);
                    break;
                }
            }
            if (FD_ISSET(client2_fd, &readfds)) {
                int valread = read(client2_fd, buffer, BUFFER_SIZE);
                buffer[valread] = '\0';
                if (strcmp(buffer, "y\n") == 0) {
                    player2_continue = 1;
                } else {
                    player2_continue = 0;
                    send(client1_fd, "Player 2 does not want to play again. Exiting...\n", 49, 0);
                    close(client1_fd);
                    close(client2_fd);
                    break;
                }
            }

            // If both players want to play again, reset the game
            if (player1_continue && player2_continue) {
                resetGame(board, &turn, &game_active);
                send(client1_fd, "Starting new game...\n", 21, 0);
                send(client2_fd, "Starting new game...\n", 21, 0);
                resetGame(board, &turn, &game_active);
                char* boardState = condenseBoard(board);
                send(client1_fd, boardState, strlen(boardState), 0);
                send(client2_fd, boardState, strlen(boardState), 0);
                free(boardState);
                send(client1_fd, "Your turn! Enter row and column (e.g., 1 1): ", 45, 0);
                send(client2_fd, "Waiting for Player 1 (X) to make a move...\n", 43, 0);
            }
        }
    }

    close(server_fd);
    return 0;
}

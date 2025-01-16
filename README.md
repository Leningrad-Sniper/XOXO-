# XOXO - Networked Multiplayer Tic-Tac-Toe in C

A C-based implementation of multiplayer Tic-Tac-Toe using both TCP and UDP networking protocols. The project demonstrates socket programming concepts through a classic game.

## Prerequisites

  GCC compiler  
  UNIX-like operating system (Linux/MacOS)  
  Basic understanding of terminal usage

## Compilation

Use the following commands to compile the programs:

// TCP Version  

    gcc tcp/server.c -o tcp_server  
    gcc tcp/client.c -o tcp_client  

//UDP Version  

    gcc udp/server.c -o udp_server  
    gcc udp/client.c -o udp_client

## Usage 

### TCP Version 

1. Start the Server
   
       ./tcp_server

2.In separate terminals, start two client instances:  
  
    ./tcp_client   

### UDP Version

1. Start the Server

       ./udp_server

2. In separate terminals, start two client instances:

       ./udp_client

## Implementation Details

### Common Features

3x3 game board representation  
Turn-based gameplay  
Input validation  
Win/Draw detection  
Game state management  

### TCP Implementation

Uses SOCK_STREAM socket type  
Connection-oriented communication  
Reliable data delivery  
Built-in order preservation  
Uses following functions:  
  accept()  
  connect()  
  send()/recv()  
  select() for multiple client handling  

### UDP Implementation 

Uses SOCK_DGRAM socket type  
Connectionless communication  
Lightweight communication  
Uses following functions: 
  sendto()/recvfrom()  

Implements client tracking using addresses  

## Game Protocol 

### Message Types  

#### Connection Messages 

Initial client connection  
Player assignment (X/O)  
Game start notification  

#### Game Messages 

Board state updates  
Move commands (row column format)  
Turn notifications  
Invalid move messages  

#### Game End Messages  

Win announcements  
Draw announcements  

## Error Handling 

The implementation includes handling for:  


Socket creation failures  
Connection errors  
Invalid moves  
Disconnection scenarios  
Buffer overflow prevention  
Input validation  

## Technical Details 

### Memory Management 

Fixed-size buffers (1024 bytes)  
Dynamic memory allocation for board state  
Proper cleanup on program exit  

### Network Configuration 

Default port: 8080  
Localhost (127.0.0.1) for testing  
IPv4 addressing








  
   

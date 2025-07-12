#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int main() {
    // Creating a socket
    int net_socket;
    net_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Specifying an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(net_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connection_status == -1) {
        printf("There was an error making a connection to the remote socket.\n");
    }

    int packet_count = 0;
    int c;
    initscr();
    noecho();

    // Send input
    while(1) {
        c = getch();
        send(net_socket, &c, sizeof(c), 0);
        if (c != ERR) { // As long as the input is not nothing
            clear();
            printw("Packet %d transmitted.\n", ++packet_count); // Counts Packets
        }
    
        if (c == 'q') {
            clear();
            break; // Exit loop if client presses q
        }
    }
    endwin();
    printf("Number of data packets transmitted: %d\n", packet_count);
    
    // Closing the socket
    close(net_socket);
   

    return 0;
}
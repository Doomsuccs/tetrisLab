#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define packet_loss_ratio 0.3

// Task 1
typedef struct {
    int seq;
    char input;
    int ack;
} Frame;

// Task 3
int simulate_loss(void) {
    return ((double)rand() / (double)RAND_MAX < packet_loss_ratio);
}

int main() {
    // Creating a socket
    int net_socket = socket(AF_INET, SOCK_DGRAM, 0);
    
    // Specifying an address for the socket
    struct sockaddr_in server_address;
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8090);
    server_address.sin_addr.s_addr = INADDR_ANY;

    Frame send_frame, received_frame;
    initscr();
    noecho();
    cbreak();
    srand(time(NULL));


    // Info
    int p_trans = 0;
    int p_retrans = 0;
    int p_success = 0;
    int p_lost = 0;
    int acks_received = 0;
    int timeouts = 0;

    int seq = 0;
    int server_len = sizeof(server_address);
    char GameOn = TRUE;
    char c;

    // Send packets
    while(GameOn) {
        int acked = 0;
        if ((c = getch()) != ERR) { // As long as the input is not nothing
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') { // Only goes through code with correct inputs (w,a,s,d,q)
                clear();
                send_frame.seq = seq;
                send_frame.input = c;
                send_frame.ack = 0;
                printw("Packet %d transmitted.\n", seq); // Counts Packets
                refresh();
                p_trans++;
                p_retrans++;
                p_success++;

                // Task 3
                if (simulate_loss() == 0) {
                    sendto(net_socket, &send_frame, sizeof(Frame), 0, (const struct sockaddr *) &server_address, server_len);
                    printw("Packet %d successfully transmitted with %lu data bytes\n", seq, sizeof(send_frame));
                    refresh();
                }
                else {
                    printw("Packet %d lost\n", seq);
                    refresh();
                    p_lost++;
                }
            }
            else if (c == 'q') { // Exit loop if client presses q
                clear();
                GameOn = FALSE; 
                continue; 
            }
            else {
                continue;
            }
        }
                
        // Task 4
        while(acked == 0) {
            // Setup timeout
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000; // 0.1 second

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(net_socket, &read_fds);
            int timeout_status = select(net_socket + 1, &read_fds, NULL, NULL, &timeout);
            if (timeout_status == -1) {
                perror("Select Error");
                GameOn = FALSE;
                continue;
            }
            else if (timeout_status == 0) {
                printw("Timeout expired for packet numbered %d\n", seq);
                printw("Packet %d generated for re-transmission\n", seq);
                p_retrans++;
                timeouts++;

                sendto(net_socket, &send_frame, sizeof(Frame), 0, (const struct sockaddr *) &server_address, server_len);
                printw("Packet %d successfully transmitted with %lu data bytes\n", seq, sizeof(send_frame));
                refresh();
                p_success++;
                continue;
            }
            else {
                recvfrom(net_socket, &received_frame, sizeof(Frame), 0, NULL, NULL);
                if (received_frame.seq == seq + 1 && received_frame.ack == 1) { // ACK received
                    printw("ACK %d received\n", received_frame.seq);
                    refresh();
                    acks_received++;
                }
                else {
                    continue;
                }
                acked = 1;
            }
        }
        seq++;
    }

    close(net_socket);
    
    endwin();
    
    // Output of statistics
    printf("Number of data packets generated for transmission: %d\n", p_trans);
    printf("Total number of data packets generated for retransmission: %d\n", p_retrans);
    printf("Number of data packets dropped due to loss: %d\n", p_lost);
    printf("Number of data packets transmitted successfully: %d\n", p_success);
    printf("Number of ACKs received: %d\n", acks_received);
    printf("Count of how many times the timeout expired: %d\n", timeouts);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>

#define ROWS 20 // you can change height and width of table with ROWS and COLS 
#define COLS 15
#define TRUE 1
#define FALSE 0

char Table[ROWS][COLS] = {0};
int score = 0;
char GameOn = TRUE;
suseconds_t timer = 400000; // decrease this to make it faster
int decrease = 1000;

typedef struct {
    char **array;
    int width, row, col;
} Shape;
Shape current;

// Task 1
typedef struct {
    int seq;
    char input;
    int ack;
} Frame;

const Shape ShapesArray[7]= {
	{(char *[]){(char []){0,1,1},(char []){1,1,0}, (char []){0,0,0}}, 3},                           //S shape     
	{(char *[]){(char []){1,1,0},(char []){0,1,1}, (char []){0,0,0}}, 3},                           //Z shape     
	{(char *[]){(char []){0,1,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //T shape     
	{(char *[]){(char []){0,0,1},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //L shape     
	{(char *[]){(char []){1,0,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //flipped L shape    
	{(char *[]){(char []){1,1},(char []){1,1}}, 2},                                                 //square shape
	{(char *[]){(char []){0,0,0,0}, (char []){1,1,1,1}, (char []){0,0,0,0}, (char []){0,0,0,0}}, 4} //long bar shape
	// you can add any shape like it's done above. Don't be naughty.
};

Shape CopyShape(Shape shape){
	Shape new_shape = shape;
	char **copyshape = shape.array;
	new_shape.array = (char**)malloc(new_shape.width*sizeof(char*));
    int i, j;
    for(i = 0; i < new_shape.width; i++){
		new_shape.array[i] = (char*)malloc(new_shape.width*sizeof(char));
		for(j=0; j < new_shape.width; j++) {
			new_shape.array[i][j] = copyshape[i][j];
		}
    }
    return new_shape;
}

void DeleteShape(Shape shape){
    int i;
    for(i = 0; i < shape.width; i++){
		free(shape.array[i]);
    }
    free(shape.array);
}

int CheckPosition(Shape shape){ //Check the position of the copied shape
	char **array = shape.array;
	int i, j;
	for(i = 0; i < shape.width;i++) {
		for(j = 0; j < shape.width ;j++){
			if((shape.col+j < 0 || shape.col+j >= COLS || shape.row+i >= ROWS)){ //Out of borders
				if(array[i][j]) //but is it just a phantom?
					return FALSE;
				
			}
			else if(Table[shape.row+i][shape.col+j] && array[i][j])
				return FALSE;
		}
	}
	return TRUE;
}

void SetNewRandomShape(){ //updates [current] with new shape
	Shape new_shape = CopyShape(ShapesArray[rand()%7]);

    new_shape.col = rand()%(COLS-new_shape.width+1);
    new_shape.row = 0;
    DeleteShape(current);
	current = new_shape;
	if(!CheckPosition(current)){
		GameOn = FALSE;
	}
}

void RotateShape(Shape shape){ //rotates clockwise
	Shape temp = CopyShape(shape);
	int i, j, k, width;
	width = shape.width;
	for(i = 0; i < width ; i++){
		for(j = 0, k = width-1; j < width ; j++, k--){
				shape.array[i][j] = temp.array[k][i];
		}
	}
	DeleteShape(temp);
}

void WriteToTable(){
	int i, j;
	for(i = 0; i < current.width ;i++){
		for(j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Table[current.row+i][current.col+j] = current.array[i][j];
		}
	}
}

void RemoveFullRowsAndUpdateScore(){
	int i, j, sum, count=0;
	for(i=0;i<ROWS;i++){
		sum = 0;
		for(j=0;j< COLS;j++) {
			sum+=Table[i][j];
		}
		if(sum==COLS){
			count++;
			int l, k;
			for(k = i;k >=1;k--)
				for(l=0;l<COLS;l++)
					Table[k][l]=Table[k-1][l];
			for(l=0;l<COLS;l++)
				Table[k][l]=0;
			timer-=decrease--;
		}
	}
	score += 100*count;
}

void PrintTable(){
	char Buffer[ROWS][COLS] = {0};
	int i, j;
	for(i = 0; i < current.width ;i++){
		for(j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Buffer[current.row+i][current.col+j] = current.array[i][j];
		}
	}
	clear();
	for(i=0; i<COLS-9; i++)
		printw(" ");
	printw("Covid Tetris\n");
	for(i = 0; i < ROWS ;i++){
		for(j = 0; j < COLS ; j++){
			printw("%c ", (Table[i][j] + Buffer[i][j])? '#': '.');
		}
		printw("\n");
	}
	printw("\nScore: %d\n", score);
}

void ManipulateCurrent(int action){
	Shape temp = CopyShape(current);
	printf("%d", action);
	switch(action){
		case 's':
			temp.row++;  //move down
			if(CheckPosition(temp))
				current.row++;
			else {
				WriteToTable();
				RemoveFullRowsAndUpdateScore();
                SetNewRandomShape();
			}
			break;
		case 'd':
			temp.col++;  //move right
			if(CheckPosition(temp))
				current.col++;
			break;
		case 'a':
			temp.col--;  //move left
			if(CheckPosition(temp))
				current.col--;
			break;
		case 'w':
			RotateShape(temp); // rotate clockwise
			if(CheckPosition(temp))
				RotateShape(current);
			break;
	}
	DeleteShape(temp);
	PrintTable();
}

struct timeval before_now, now;
int hasToUpdate(){
	return ((suseconds_t)(now.tv_sec*1000000 + now.tv_usec) -((suseconds_t)before_now.tv_sec*1000000 + before_now.tv_usec)) > timer;
}

int main() {
    // Creating the server socket
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0); // Use SOCK_DGRAM for UDP

    // Define the server address
    struct sockaddr_in server_address, client_address;
    memset(&server_address, '\0', sizeof(server_address));
	memset(&client_address, '\0', sizeof(client_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8090);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Make the server socket non-blocking
    int block_status = fcntl(server_socket, F_GETFL, 0);
    fcntl(server_socket, F_SETFL, block_status | O_NONBLOCK);

    // Bind the socket to our IP and port
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));   
    printf("Server listening...\n");

    int client_len = sizeof(client_address);
    srand(time(0));
    score = 0;
    initscr();
	gettimeofday(&before_now, NULL);
	timeout(1);
    Frame received_frame, send_frame;
    char start = FALSE;

	SetNewRandomShape();
    PrintTable();

	while(GameOn){
		int status = recvfrom(server_socket, &received_frame, sizeof(Frame), 0, (struct sockaddr *) &client_address, &client_len);
		if (status < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // Task 2
                if (start != TRUE) { // wait until a key is pressed to begin game
                    continue;
                }

				refresh(); 
				gettimeofday(&now, NULL);
				if (hasToUpdate()) { // time difference in microsec accuracy
					ManipulateCurrent('s');
					gettimeofday(&before_now, NULL);
				}
				continue;
			}
		}
        
        // Task 2
        // Client has pressed a key so the game can start
        if (send_frame.seq >= 0) {
            start = TRUE;
        }

		send_frame.seq = received_frame.seq + 1;
		send_frame.ack = 1;
		send_frame.input = received_frame.input;

		ManipulateCurrent(received_frame.input);
		refresh();

		sendto(server_socket, &send_frame, sizeof(Frame), 0, (const struct sockaddr *) &client_address, client_len);
	}
	DeleteShape(current);
	endwin();
	int i, j;
	for(i = 0; i < ROWS ;i++){
		for(j = 0; j < COLS ; j++){
			printf("%c ", Table[i][j] ? '#': '.');
		}
		printf("\n");
	}
	printf("\nGame ouvre!\n");
	printf("\nScore: %d\n", score);

    close(server_socket);

    return 0;
}

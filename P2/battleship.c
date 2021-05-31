#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SHIP{
    int row, column, length;
    char direction;
} SHIP;

typedef struct ATTACK{
    int row, column;
} ATTACK;

// these are the prototype functions developed for our solution.  You are welcome to use them 
// or modify them if you have a different strategy for solving this project.
void Read_Save_Game_File(char *filename, int *rows, int *columns, int *ship_count, SHIP **ships, int *attack_count, ATTACK **attacks);
void Initialize_Board(int rows, int columns, char **board);
void Print_Board(int rows, int columns, char *board);
void Add_Ships(int rows, int columns, char *board, int ship_count, SHIP *ships);
void Process_Attacks(int rows, int columns, char *board, int attack_count, ATTACK *attacks);
void Game_Over(int rows, int columns, char *board);

int main(int argc, char *argv[] ) {

    // verify command line parameters
    if(argc != 2){
        printf("expected usage: %s <save_game_file>", argv[0]);
        exit(1);
    }

    // declare variables - feel free to add additional variables as needed
    char *board = NULL;
    SHIP *ships = NULL;
    ATTACK *attacks = NULL;
    int rows=0, columns=0, ship_count=0, attack_count=0;
    
    // read the save game file
    Read_Save_Game_File(argv[1], &rows, &columns, &ship_count, &ships, &attack_count, &attacks);
    
    /* Note: Reading the file and using malloc to reserve space for 
     * the data may be easier to do here, in main, rather than in a 
     * function */

    // Generate the board;
    Initialize_Board(rows, columns, &board);
    Print_Board(rows, columns, board);
    
	
    // Add Ships to the board;
    Add_Ships(rows, columns, board, ship_count, ships);
    Print_Board(rows, columns, board);
	
    // Process Attacks
    Process_Attacks(rows, columns, board, attack_count, attacks);
    Print_Board(rows, columns, board);
	
    Game_Over(rows, columns, board);
    
    // free memory
    free(board);
    free(ships);
    free(attacks);
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reads the save game file which contains the rows, columns, number of ships, ship positions, number of attacks, and attack locations.
// Read the relevant variables and stores them in the heap & stack.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Read_Save_Game_File(char *filename, int *rows, int *columns, int *ship_count, SHIP **ships, int *attack_count, ATTACK **attacks) {
    printf("Reading Save Game File\n");
    FILE *input = fopen(filename, "r");
    if(!input){
        printf("INPUT File open failed\n");
        exit(1);
    }
    
    // skip the board size line
    char skip_this_text[1000];\
    if (!fgets(skip_this_text, 1000, input)) {
        printf("Error reading board size");
        exit(1);}

    // read the board size
    fscanf(input, "%d", rows);
    fscanf(input, "%d", columns);
    fgets(skip_this_text, 1000, input); // read the newline character
    printf("Board Size = (%d, %d)\n",*rows, *columns);

    /*  Complete the rest of the function below - or move the code above to main */
    // skip the number of ships line
    if (!fgets(skip_this_text, 1000, input)) {
        printf("Error reading number of ships");
        exit(1);}
    
    /// read the number of ships
    fscanf(input, "%d", ship_count);
    fgets(skip_this_text, 1000, input); // read the newline character
    printf ("Number of Ships = %d\n", *ship_count);
    
	// read position, length, direction of ships and store in heap
	*ships = malloc((*ship_count) * sizeof(SHIP));
	
	for (int x = 0; x < *ship_count; x++) {
		fscanf(input, "%d", &(*ships)[x].row);
		fscanf(input, "%d", &(*ships)[x].column);
		fscanf(input, "%d", &(*ships)[x].length);
		fscanf(input, "%s", &(*ships)[x].direction);
		printf ("Ship %d: (%d, %d), length = %d, direction %c\n", x, (*ships)[x].row, (*ships)[x].column, (*ships)[x].length, (*ships)[x].direction);
		fgets(skip_this_text, 1000, input); // read the newline character
	}

	// skip the number of attacks line	
	if (!fgets(skip_this_text, 1000, input)) {
        printf("Error reading number of attacks");
        exit(1);}
    
    // read number of attacks
	fscanf (input, "%d", attack_count);
	fgets(skip_this_text, 1000, input); // read the newline character
	printf ("Number of Attacks = %d\n", *attack_count);
	
	// read position, length, direction of ships and store in heap
	*attacks = malloc((*attack_count) * sizeof(ATTACK));
	
	for (int x = 0; x < *attack_count; x++) {
		fscanf(input, "%d", &(*attacks)[x].row);
		fscanf(input, "%d", &(*attacks)[x].column);
		printf ("Attack %d: (%d, %d)\n", x, (*attacks)[x].row, (*attacks)[x].column);
		fgets(skip_this_text, 1000, input); // read the newline character
	}
	
	printf ("\n");
    fclose(input);
}

///////////////////////////////////////
// Initializes the initial empty board
//////////////////////////////////////
void Initialize_Board(int rows, int columns, char **board) {
	printf ("Initializing Board\n");
	*board = malloc (rows*columns*sizeof(char));

	// loops throughout the board and initializes '.' to each address
	for ( int x = 0; x < (rows*columns); x++) {
		*(*board + x) = '.';
	}
	
}

//////////////////////////////////////////////////////////////
// Prints the board stored in the heap in rows * column format 
//////////////////////////////////////////////////////////////
void Print_Board(int rows, int columns, char *board) {
	
	// loops throughout the board and prints
	for (int x = 0; x < rows; x++) {
		for ( int y = 0; y < columns; y++) {
			printf ("%c", *board);
			board++;
		}
		printf ("\n");	
	}
	printf ("\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Adds the ships to the board based on their location and their eligibility in terms of collision and bounds. 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Add_Ships(int rows, int columns,char *board, int ship_count, SHIP *ships) {
	printf ("Adding Ships\n");
	char *board_orig = board;
	
	for (int x = 0; x < ship_count; x++) {
		int valid = 1;
		
		// if ship is horizontal
		if (ships[x].direction == 'H') {
			// if out of bounds (negative indices)
			if (ships[x].row < 0 || ships[x].column < 0) {
				printf ("Ship %d is out of bounds - skipping\n", x);
				valid = 0;
			}
			// if out of bounds (too long)
			else if (((ships[x].column) + (ships[x].length)) > columns) {
				printf ("Ship %d is out of bounds - skipping\n", x);
				valid = 0;
			}
			// if collision
			else {
				board = board + ((ships[x].row) * columns) + (ships[x].column);
				for ( int y = 0; y < ships[x].length; y++) {
					if ((*board) != '.') {
						printf ("Ship %d overlaps another ship - skipping\n", x);
						valid = 0;
						break;
					}
					board++;
				}
				board = board_orig;
			}
		}
		
		// if ship is vertical
		else if (ships[x].direction == 'V') {
			// if out of bounds (negative indices)
			if (ships[x].row < 0 || ships[x].column < 0) {
				printf ("Ship %d is out of bounds - skipping\n", x);
				valid = 0;
			}
			// if out of bounds (too long)
			else if (((ships[x].row) + (columns * (ships[x].length))) > (rows * columns)) {
				printf ("Ship %d is out of bounds - skipping\n", x);
				valid = 0;
			}
			// if collision
			else {
				board = board + ((ships[x].row) * columns) + (ships[x].column);
				for ( int y = 0; y < ships[x].length; y++) {
					//printf ("%d\n", y);
					if ((*board) != '.') {
						printf ("Ship %d overlaps another ship - skipping\n", x);
						valid = 0;
						break;
					}
					board += columns;
				}
				board = board_orig;
			}
		}
		
		// if ship is valid to add
		if (valid) {
			// if ship is horizontal
			if (ships[x].direction == 'H') {
				board = board + ((ships[x].row) * columns) + (ships[x].column);
				for ( int y = 0; y < ships[x].length; y++) {
					*board = 'S';
					board++;
				}
				board = board_orig;
			}
			
			// if ship is vertical
			else if (ships[x].direction == 'V') {
				board = board + ((ships[x].row) * columns) + (ships[x].column);
				for ( int y = 0; y < ships[x].length; y++) {
					*board = 'S';
					board += columns;
				}
				board = board_orig;
			}
		}	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Updates the board with indications on whether the attack is successful or not and also determines the eligibility based on its bounds 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Process_Attacks(int rows, int columns,char *board,int attack_count,ATTACK *attacks) {
	printf ("Processing Attacks\n");
	char *board_orig = board;

	// loops through every attack
	for (int x = 0; x < attack_count; x++) {
		int valid = 1;
		// points to the attack position
		board = board + ((attacks[x].row) * columns) + (attacks[x].column);
		
		// if attack is out of bounds
		if (attacks[x].row < 0 || attacks[x].column < 0 || attacks[x].row >= rows || attacks[x].column >= columns) {
			printf ("Attack %d is out of bounds - skipping\n", x);
			valid = 0;
		}
		
		// if HIT
		if (*board == 'S' && valid) {
			*board = 'H';
		}
		
		// if MISS
		else if (*board == '.' && valid) {
			*board = 'M';
		}
		
		// resets pointer
		board = board_orig;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Notifies the user if the game is over by detecting the presence of a ship afloat on the board 
////////////////////////////////////////////////////////////////////////////////////////////////
void Game_Over(int rows, int columns, char *board) {
	int gameon = 0;
	
	// loops through every point on the board
	for (int x = 0; x < rows*columns; x++) {
		// if there exists a ship afloat
		if (*(board + x) == 'S'){
			printf ("Ships remain afloat - game on\n");
			gameon = 1;
			break;
		}
	}
	
	// if no ship afloat
	if (!gameon) {
		printf ("All ships have been sunk - game over\n");
	}
}

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#define GRID_ROWS 6
#define GRID_COLS 7

typedef enum { BOT, USER } PLAYER;
typedef enum { KEY_NONE, LEFT_ARROW, RIGHT_ARROW, KEY_ENTER } Key;

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"

// ---------------- CURSOR ----------------
void hideCursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void showCursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = TRUE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void moveCursorTop() {
	printf("\033[H");
}

PLAYER pickRandomPlayer() {
	return rand() % 2 == 0 ? BOT : USER;
}

void fillDefaultGrid(char grid[GRID_ROWS][GRID_COLS]) {
	for (int i = 0; i < GRID_ROWS; i++)
		for (int j = 0; j < GRID_COLS; j++)
			grid[i][j] = '#';
}

void displayGrid(char grid[GRID_ROWS][GRID_COLS]) {
	for (int row = 0; row < GRID_ROWS; row++) {
		printf("|");
		for (int col = 0; col < GRID_COLS; col++) {
			char c = grid[row][col];
			if (c == '*') printf(" %s%c%s ", COLOR_RED, c, COLOR_RESET);
			else if (c == '0') printf(" %s%c%s ", COLOR_YELLOW, c, COLOR_RESET);
			else printf(" %c ", c);
		}
		printf("|\n");
	}
	printf("+");
	for (int col = 0; col < GRID_COLS; col++)
		printf("---");
	printf("+\n");
	printf("\n");
}

void displayCursor(int currentCol, PLAYER player_turn) {
	printf(" ");
	for (int col = 0; col < GRID_COLS; col++) {
		if (col == currentCol) {
			if (player_turn == USER)
				printf(" %s*%s ", COLOR_RED, COLOR_RESET);
			else
				printf(" %s0%s ", COLOR_YELLOW, COLOR_RESET);
		}
		else printf("   ");
	}
	printf("\n");
}

// ---------------- INPUT ----------------
Key getKey() {
	int ch = _getch();
	if (ch == 224) {
		ch = _getch();
        switch (ch) { case 75: return LEFT_ARROW; case 77: return RIGHT_ARROW; default: return KEY_NONE; }
	}
	if (ch == 13) return KEY_ENTER;
	return KEY_NONE;
}

bool selectCol(int* currentCol) {
	Key k = getKey();
	switch (k) {
	case LEFT_ARROW: *currentCol = (*currentCol - 1 + GRID_COLS) % GRID_COLS; break;
	case RIGHT_ARROW: *currentCol = (*currentCol + 1) % GRID_COLS; break;
	case KEY_ENTER: return true;
	default: break;
	}
	return false;
}

// ---------------- ANIMATION ----------------
void animateDrop(char grid[GRID_ROWS][GRID_COLS], int col, PLAYER p) {
	char symbol = (p == USER) ? '*' : '0';

	for (int row = 0; row < GRID_ROWS; row++) {
		if (grid[row][col] != '#') break;

		grid[row][col] = symbol;

		moveCursorTop();
		displayCursor(col, p);
		displayGrid(grid);

		if (row < GRID_ROWS - 1) grid[row][col] = '#';

		Sleep(50);
	}
}

bool placePiece(char grid[GRID_ROWS][GRID_COLS], int col, PLAYER p) {
	for (int row = GRID_ROWS - 1; row >= 0; row--) {
		if (grid[row][col] == '#') {
			animateDrop(grid, col, p);
			grid[row][col] = (p == USER) ? '*' : '0';
			return true;
		}
	}
	return false;
}

// ---------------- WIN / TIE ----------------
bool checkWin(char grid[GRID_ROWS][GRID_COLS], PLAYER p) {
	char c = p == USER ? '*' : '0';
	for (int r = 0; r < GRID_ROWS; r++)
		for (int col = 0; col <= GRID_COLS - 4; col++)
			if (grid[r][col] == c && grid[r][col + 1] == c && grid[r][col + 2] == c && grid[r][col + 3] == c)
				return true;
	for (int cCol = 0; cCol < GRID_COLS; cCol++)
		for (int row = 0; row <= GRID_ROWS - 4; row++)
			if (grid[row][cCol] == c && grid[row + 1][cCol] == c && grid[row + 2][cCol] == c && grid[row + 3][cCol] == c)
				return true;
	for (int row = 0; row <= GRID_ROWS - 4; row++)
		for (int col = 0; col <= GRID_COLS - 4; col++)
			if (grid[row][col] == c && grid[row + 1][col + 1] == c && grid[row + 2][col + 2] == c && grid[row + 3][col + 3] == c)
				return true;
	for (int row = 0; row <= GRID_ROWS - 4; row++)
		for (int col = 3; col < GRID_COLS; col++)
			if (grid[row][col] == c && grid[row + 1][col - 1] == c && grid[row + 2][col - 2] == c && grid[row + 3][col - 3] == c)
				return true;
	return false;
}

bool checkTie(char grid[GRID_ROWS][GRID_COLS]) {
	for (int col = 0; col < GRID_COLS; col++)
		if (grid[0][col] == '#') return false;
	return true;
}

// ---------------- BOT ----------------
int firstEmptyRow(char grid[GRID_ROWS][GRID_COLS], int col) {
	for (int row = GRID_ROWS - 1; row >= 0; row--)
		if (grid[row][col] == '#') return row;
	return -1;
}

bool willWin(char grid[GRID_ROWS][GRID_COLS], int col, PLAYER p) {
	int row = firstEmptyRow(grid, col);
	if (row == -1) return false;
	grid[row][col] = p == USER ? '*' : '0';
	bool win = checkWin(grid, p);
	grid[row][col] = '#';
	return win;
}

void botPlay(char grid[GRID_ROWS][GRID_COLS], int* currentCol) {
	int targetCol = -1;

	for (int c = 0; c < GRID_COLS; c++)
		if (willWin(grid, c, BOT)) { targetCol = c; break; }

	if (targetCol == -1)
		for (int c = 0; c < GRID_COLS; c++)
			if (willWin(grid, c, USER)) { targetCol = c; break; }

	if (targetCol == -1) {
		int available[GRID_COLS];
		int count = 0;
		for (int c = 0; c < GRID_COLS; c++)
			if (firstEmptyRow(grid, c) != -1)
				available[count++] = c;
		if (count > 0)
			targetCol = available[rand() % count];
	}

	while (*currentCol != targetCol) {
		if (*currentCol < targetCol) (*currentCol)++;
		else (*currentCol)--;

		moveCursorTop();
		displayCursor(*currentCol, BOT);
		displayGrid(grid);
		Sleep(100 + rand() % 100);
	}

	Sleep(300 + rand() % 500);
	placePiece(grid, *currentCol, BOT);
}

// ---------------- ENDGAME ----------------
bool handleEndGame(char grid[GRID_ROWS][GRID_COLS], int winner) {
	system("cls");
	displayGrid(grid);
	if (winner == USER) printf("You WIN!\n");
	else if (winner == BOT) printf("Bot WINS!\n");
	else printf("TIE!\n");

	printf("Press R to restart or any key to exit.\n");
	char c = _getch();
	if (c == 'R' || c == 'r') {
		fillDefaultGrid(grid);
		return true;
	}
	return false;
}

void displayTurnMessage(const char* msg) {
	// BRING THE LINE ON TOP AND CLEAR IT
	printf("\033[F\033[2K");
	printf("%s\n", msg);
}


// ---------------- MAIN ----------------
int main() {
	srand(time(NULL));
	hideCursor();

	char grid[GRID_ROWS][GRID_COLS];
	int playerCol = 0;
	PLAYER turn;

	fillDefaultGrid(grid);
	turn = pickRandomPlayer();

	while (true) {
		moveCursorTop();
		displayCursor(playerCol, turn);
		displayGrid(grid);

		if (turn == USER) {
			displayTurnMessage("Your turn!\nUse left arrow | right arrow and ENTER to interact.");

			bool moveDone = false;
			while (!moveDone) {
				moveCursorTop();
				displayCursor(playerCol, USER);
				displayGrid(grid);
				if (selectCol(&playerCol) && placePiece(grid, playerCol, USER)) {
					moveDone = true;
				}
			}

			if (checkWin(grid, USER)) {
				if (handleEndGame(grid, USER)) { turn = pickRandomPlayer(); continue; }
				break;
			}
			else if (checkTie(grid)) {
				if (handleEndGame(grid, -1)) break;
			}

			displayTurnMessage("");
			turn = BOT;
		}
		else
		{
			displayTurnMessage("Bot thinking...");
			Sleep(300 + rand() % 200);
			botPlay(grid, &playerCol);

			displayTurnMessage("");
			if (checkWin(grid, BOT)) {
				if (handleEndGame(grid, BOT)) { turn = pickRandomPlayer(); continue; }
				break;
			}
			else if (checkTie(grid)) {
				if (handleEndGame(grid, -1)) break;
			}

			turn = USER;
		}
	}


	showCursor();
	return 0;
}


/*
	That was the initial plan I had made, but then I changed it after getting used to AI to make it more fun.
		Console Visual :
		- Show the grid 6 * 7 filled with empty by default
		- Show # for empty, * for the user, and 0 for the bot
		- Show the indices from 0 to 6 on top of the grid to let the user know which column to pick
		- Show "Your turn. Choose a column (0–6) or type 'exit' to leave:" when it's the user's turn to place the coin

Interaction :
	- Pick randomly who starts first (user or bot)
	- Don't show immediately when the bot places a coin. wait 1–3 seconds before updating the grid
	- Ask the user to pick a column between 0–6 (left to right)

Error :
	- The user can only write 0 to 6 otherwise we simply clear the output and ask back to pick a cols from 0 to 6 or type exit to leave.

Bot :
	- Check from bottom row to top row to find the first empty cell.
	- Prioritize moves as follows:
		- If the bot can win immediately in any column, play that column
		- If the player could win in the next move, block that column
		- If neither, play the center column (3) if available
			- Otherwise, play the column closest to the center something like : (2, 4, 1, 5, 0, 6)

Condition to check before setting a coin to the selected column :
	- From bottom to top, check if a coin is already set
	- If the row is not the top one and the cell is occupied, recursively - 1 check the row above
	- If a free place is found, set the coin there and exit the function
	- If the column is full, do nothing (exit !!!)

Condition to check after setting a coin to the selected column :
	- Check for 4 aligned coins horizontally, vertically, main diagonal, or secondary diagonal
	- If any alignment is found, display the winner or a tie and Ask the user to press R to restart the game
		- if R is typed then we reset the GRID and redo the process again.
*/
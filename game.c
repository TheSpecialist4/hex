#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/**
    Exit codes for error conditions
**/
typedef enum {
    OK = 0,
    USAGE = 1,
    PLAYER_TYPE = 2,
    GRID_DIMENSIONS = 3,
    FILE_READ = 4,
    INVALID_FILE = 5,
    EOF_ERROR = 6
} ErrorCode;

/**
    Contains the information about a player in the game
**/
typedef struct Player {
    bool isManual; // true if the player is manual
    int moveCounter;
    char playerName;
} Player;

/**
    Contains the information about the game
**/
typedef struct Game {
    int height;
    int width;
    char** board;
    Player* players[2];
    bool isXTurn; // true if the currently player playing is X
    char winner;
} Game;

/**
    This data structure is used to determine the end game conditions.
**/
typedef struct Stack {
    // the row values of the cells on the game board
    int* rows;
    // the column values of the cells
    int* columns;
    // a log of all the visited cell row values
    int* rowLog;
    // log of all the visited cell column values
    int* columnLog;
    // number of cells visited
    int logCount;
    // maximum size of the log array
    int logSize;
    // the number of elements currently in the stack
    int count;
    // capacity of the stack
    int maxSize;
} Stack;

char** split_string(char*, int*, char*);

void free_game(Game*);

void free_stack(Stack*);

/**
    Initializes the game using the given height and width parameters
    as the game board dimensions
**/
Game* initialize_game(int height, int width) {
    Game* game = malloc(sizeof(Game));
    game->height = height;
    game->width = width;
    game->isXTurn = false;
    game->board = malloc(sizeof(char*) * height);
    for (int i = 0; i < height; i++) {
        game->board[i] = malloc(sizeof(char) * width);
        memset(game->board[i], '.', width);
    }

    game->players[0] = malloc(sizeof(Player));
    game->players[1] = malloc(sizeof(Player));
    game->players[0]->playerName = 'O';
    game->players[1]->playerName = 'X';
    game->winner = '.';

    return game;
}

/**
    Initializes the player in the game and allocates appropriate memory
**/
void initialize_player(char* playerType, Player* player, int moves) {
    player->moveCounter = moves;
    if (playerType[0] == 'm') {
        player->isManual = true;
    } else {
        player->isManual = false;
    }
}

/**
    Initializes the stack used for game end detection
**/
Stack* initialize_stack(Game* game) {
    Stack* stack = malloc(sizeof(Stack));
    stack->rows = malloc(sizeof(int) * (game->height * game->width));
    stack->columns = malloc(sizeof(int) * (game->height * game->width));
    stack->rowLog = malloc(sizeof(int) * (game->height * game->width));
    stack->columnLog = malloc(sizeof(int) * (game->height * game->width));
    stack->count = 0;
    stack->logCount = 0;
    stack->logSize = game->height * game->width;
    stack->maxSize = game->height * game->width;

    return stack;
}

/**
    Prints the game board
**/
void print_game(Game* game) {
    for (int i = 0; i < game->height; i++) {
        for (int j = 0; j < (game->height) - 1 - i; j++) {
            printf(" ");
        }
        for (int j = 0; j < game->width; j++) {
            printf("%c", game->board[i][j]);
            if (j < game->width - 1) {
                printf(" ");
            }
        }
        printf("\n");
    }
}

/**
    Shows the appropriate error messages based on the ErrorCode e parameter
**/
int show_error_message(ErrorCode e) {
    const char* message = "";
    switch(e) {
        case OK:
            break;
        case USAGE:
            message = "Usage: hex p1type p2type [height width | filename]\n";
            break;
        case PLAYER_TYPE:
            message = "Invalid type\n";
            break;
        case GRID_DIMENSIONS:
            message = "Sensible board dimensions please!\n";
            break;
        case FILE_READ:
            message = "Could not start reading from savefile\n";
            break;
        case INVALID_FILE:
            message = "Incorrect file contents\n";
            break;
        case EOF_ERROR:
            message = "EOF from user\n";
            break;
    }
    fprintf(stderr, "%s", message);
    return e;
}

/**
    Reads a line from the loaded game file and appropriately
    updates the game data
**/
int add_row_from_file(char* line, int* lineCount, Game* game) {
    if (*lineCount < game->height) {
        if (line[game->width] != '\n') {
            return -1;
        }
    }
    if (*lineCount > game->height) {
        return -1;
    }
    for (int i = 0; i < game->width; i++) {
        if (line[i] != 'O' && line[i] != 'X' && line[i] != '.') {
            return -1;
        }
        game->board[*lineCount - 1][i] = line[i];
    }
    return 0;
}

/**
    Loads the game data from the file given by the parameter 'gameFile'
**/
int load_game(FILE* gameFile, Game** game) {
    char line[150];
    int tokenCount = 0, playerTurn = 0, lineIndex = 0;
    char* error = 0;
    int height, width, oMoveCount, xMoveCount;
    while (fgets(line, 145, gameFile) != NULL) {
        if (lineIndex == 0) {
            char** lineSplit = split_string(line, &tokenCount, ",");
            if (tokenCount != 5) {
                return -1;
            }
            playerTurn = (int)strtol(lineSplit[0], &error, 10);
            if (*error != '\0' || (playerTurn != 0 && playerTurn != 1)) {
                return -1;
            }
            height = (int)strtol(lineSplit[1], &error, 10);
            if (*error != '\0' || height <= 0 || height > 1000) {
                return -1;
            }
            width = (int)strtol(lineSplit[2], &error, 10);
            if (*error != '\0' || width <= 0 || width > 1000) {
                return -1;
            }
            oMoveCount = (int)strtol(lineSplit[3], &error, 10);
            if (*error != '\0' || oMoveCount < 0 || oMoveCount > 1000) {
                return -1;
            }
            lineSplit[4][strlen(lineSplit[4]) - 1] = '\0'; // \n character
            xMoveCount = (int)strtol(lineSplit[4], &error, 10);
            if (*error != '\0' || xMoveCount < 0 || xMoveCount > 1000) {
                return -1;
            }
            *game = initialize_game(height, width);
            if (playerTurn > 0) {
                (*game)->isXTurn = true;
            }
        } else {
            if (add_row_from_file(line, &lineIndex, *game) < 0) {
                return -1;
            }
        }
        lineIndex++;
    }
    initialize_player("a", (*game)->players[0], oMoveCount);
    initialize_player("a", (*game)->players[1], xMoveCount);
    return 0;
}

/**
    Adds a cell's row and column values to the stack,
    and returns the updated stack. Stack remains unchanged for
    duplicate additions.
**/
Stack* push(Stack* stack, int row, int column) {
    if (stack->count == stack->maxSize) {
        return stack;
    }
    if (stack->logCount == stack->maxSize) {
        stack->rowLog = realloc(stack->rowLog, stack->logSize * 2);
        stack->columnLog = realloc(stack->columnLog, stack->logSize * 2);
        stack->logSize *= 2;
    }
    for (int i = 0; i < stack->logCount; i++) {
        if (stack->rowLog[i] == row && stack->columnLog[i] == column) {
            return stack;
        }
    }
    stack->rowLog[stack->logCount] = row;
    stack->columnLog[stack->logCount++] = column;

    stack->rows[stack->count] = row;
    stack->columns[(stack->count)++] = column;
    return stack;
}

/**
    Removes the cell given by its row and column values from the stack
    and returns the updated stack. Stack remains unchanged if empty.
**/
Stack* pop(Stack* stack, int* row, int* column) {
    if (stack->count == 0) {
        return stack;
    }
    *row = stack->rows[--(stack->count)];
    *column = stack->columns[stack->count];
    return stack;
}

/**
    Adds all the neighbours above the current cell to the stack.
    The current cell is represented by the 'row' and 'column' parameters.
    The updated stack is returned.
**/
Stack* add_top_cells_to_stack(Stack* stack, int row, int column, 
        Game* game, char value) {
    if (column > 0) {
        if (game->board[row][column - 1] == value) {
            // left
            stack = push(stack, row, column - 1);
        }
        if (row > 0) {
            if (game->board[row - 1][column - 1] == value) {
                // top-left
                stack = push(stack, row - 1, column - 1);
            }
        }
    }
    if (row > 0) {
        if (game->board[row - 1][column] == value) {
            // top-right
            stack = push(stack, row - 1, column);
        }
    }
    if (column < game->width - 1) {
        if (game->board[row][column + 1] == value) {
            // right
            stack = push(stack, row, column + 1);
        }
    }
    return stack;
}

/**
    Adds all the cells below the current cell to the stack and returns
    the updated stack. The current cell is represented 
    by the 'row' and 'column' parameters.
**/
Stack* add_bottom_cells_to_stack(Stack* stack, int row, int column,
        Game* game, char value) {
    if (column > 0) {
        if (game->board[row][column - 1] == value) {
            // left
            stack = push(stack, row, column - 1);
        }
    }
    if (row < game->height - 1) {
        if (game->board[row + 1][column] == value) {
            // bottom left
            stack = push(stack, row + 1, column);
        }
        if (column < game->width - 1) {
            if (game->board[row + 1][column + 1] == value) {
                // bottom right
                stack = push(stack, row + 1, column + 1);
            }
        }
    }
    if (column < game->width - 1) {
        if (game->board[row][column + 1] == value) {
            // right
            stack = push(stack, row, column + 1);
        }
    }
    return stack;
}

/**
    Adds all the neighbours left of the current cell to the stack and
    returns the updated stack. The current cell
    is represented by the 'row' and 'column' parameters.
**/
Stack* add_left_cells_to_stack(Stack* stack, int row, int column,
        Game* game, char value) {
    if (column > 0) {
        if (game->board[row][column - 1] == value) {
            // left
            stack = push(stack, row, column - 1);
        }
    }
    if (row > 0 && column > 0) {
        if (game->board[row - 1][column - 1] == value) {
            // top left
            stack = push(stack, row - 1, column - 1);
        }
    }
    if ((row < game->height - 1) && game->board[row + 1][column] == value) {
        // bottom left
        stack = push(stack, row + 1, column);
    }
    return stack;
}

/**
    Adds all the neighbours right of the current cell to the stack and
    returns the updated stack. The current cell is represented by the
    'row' and 'column' parameters.
**/
Stack* add_right_cells_to_stack(Stack* stack, int row, int column,
        Game* game, char value) {
    if (column < game->width - 1) {
        if (game->board[row][column + 1] == value) {
            // right
            stack = push(stack, row, column + 1);
        }
    }
    if (row > 0) {
        if (game->board[row - 1][column] == value) {
            // top right
            stack = push(stack, row - 1, column);
        }
    }
    if (row < game->height - 1 && column < game->width - 1) {
        if (game->board[row + 1][column + 1] == value) {
            // bottom right
            stack = push(stack, row + 1, column + 1);
        }
    }
    return stack;
}

/**
    Returns true if the 'stack.pop()' element is connected to the
    top of the board, and false otherwise.
**/
bool check_top(Stack* stack, char value, Game* game) {
    while (stack->count > 0) {
        int row = -1, column = -1;
        stack = pop(stack, &row, &column);
        if (row == 0 && game->board[row][column] == value) {
            return true;
        }
        stack = add_top_cells_to_stack(stack, row, column, game, value);
    }
    return false;
}

/**
    Returns true if the 'stack.pop()' element is connected to the
    bottom of the board, and false otherwise.
**/
bool check_bottom(Stack* stack, char value, Game* game) {
    while (stack->count > 0) {
        int row = -1, column = -1;
        stack = pop(stack, &row, &column);
        if ((row == game->height - 1) && game->board[row][column] == value) {
            return true;
        }
        stack = add_bottom_cells_to_stack(stack, row, column, game, value);
    }
    return false;
}

/**
    Returns true if the 'stack.pop()' element is connected to the
    left of the board, and false otherwise.
**/
bool check_left(Stack* stack, char value, Game* game) {
    while (stack->count > 0) {
        int row = -1, column = -1;
        stack = pop(stack, &row, &column);
        if (column == 0 && game->board[row][column] == value) {
            return true;
        }
        stack = add_left_cells_to_stack(stack, row, column, game, value);
    }
    return false;
}

/**
    Returns true if the 'stack.pop()' element is connected to the
    right of the board, and false otherwise.
**/
bool check_right(Stack* stack, char value, Game* game) {
    while (stack->count > 0) {
        int row = -1, column = 1;
        stack = pop(stack, &row, &column);
        if (column == game->width - 1 && game->board[row][column] == value) {
            return true;
        }
        stack = add_right_cells_to_stack(stack, row, column, game, value);
    }
    return false;
}

/**
    Checks the game over conditions after every move and returns true if the
    game is over.
**/
bool check_game_over(int row, int column, char value, Game* game) {
    Stack* stack = initialize_stack(game);
    stack = push(stack, row, column);
    // player X, checks for top-bottom connection
    if (value == 'X') {
        if (check_top(stack, value, game)) {
            stack->count = 0;
            stack->logCount = 0;
            stack = push(stack, row, column);
            if(check_bottom(stack, value, game)) {
                game->winner = value;
                return true;
            }
        }
    } else { // player O, checks for left-right connection
        if (check_left(stack, value, game)) {
            stack->count = 0;
            stack->logCount = 0;
            stack = push(stack, row, column);
            if (check_right(stack, value, game)) {
                game->winner = value;
                return true;
            }
        }
    }
    free_stack(stack);
    return false;
}

/**
    Returns true if the move currently generated or 
    obtained from stdin is valid.
**/
bool is_move_valid(int height, int width, Game* game) {
    if (height >= game->height || width >= game->width) {
        return false;
    }
    if (height < 0 || width < 0) {
        return false;
    }
    if (game->board[height][width] != '.') {
        return false;
    }
    return true;
}

/**
    Generates the automatic move for player O
**/
void get_auto_move_for_o(int* height, int* width, Game* game) {
    // variables' names chosen to match the formula
    int m, t;
    if (game->height >= game->width) {
        m = game->height;
    } else {
        m = game->width;
    }
    t = (game->players[0]->moveCounter * 9 % 1000037) + 17;
    *height = (t / m) % game->height;
    *width = t % game->width;

    game->players[0]->moveCounter++;
}

/**
    Generates the automatic move for player X
**/
void get_auto_move_for_x(int* height, int* width, Game* game) {
    // variables' names chosen to match the formula
    int m, t;
    if (game->height >= game->width) {
        m = game->height;
    } else {
        m = game->width;
    }
    t = (game->players[1]->moveCounter * 7 % 1000213) + 81;
    *height = (t / m) % game->height;
    *width = t % game->width;

    game->players[1]->moveCounter++;
}

/**
    Saves the game currently being played
**/
void save_game(Game* game, char* fileName) {
    FILE* outputFile = fopen(fileName + 1, "w");
    if (outputFile == NULL) {
        printf("Unable to save game\n");
        return;
    } else {
        fprintf(outputFile, "%d,%d,%d,%d,%d\n", game->isXTurn, game->height,
                game->width, game->players[0]->moveCounter,
                game->players[1]->moveCounter);
        for (int i = 0; i < game->height; i++) {
            for (int j = 0; j < game->width; j++) {
                fprintf(outputFile, "%c", game->board[i][j]);
            }
            fprintf(outputFile, "\n");
        }
    }
    fflush(outputFile);
    fclose(outputFile);
}

/**
    Gets the move for the current player and returns true if
    the game is over after the move.
**/
bool get_move(Player* player, Game* game) {
    int height = -1;
    int width = -1;
    do {
        if (player->isManual) {
            printf("Player %c] ", player->playerName);
            char buffer[70];
            if (fgets(buffer, 65, stdin) == NULL) {
                exit(show_error_message(EOF_ERROR));
            }
            if (buffer[strlen(buffer) - 1] == '\n') {
                buffer[strlen(buffer) - 1] = '\0';
            }
            if (buffer[0] == 's') {
                save_game(game, buffer);
                continue;
            }
            int tokenCount = 0;
            char** line = split_string(buffer, &tokenCount, " ");
            if (tokenCount != 2) {
                continue;
            }
            char* error = 0;
            height = (int)strtol(line[0], &error, 10);
            if (*error != '\0') {
                height = -1;
            }
            width = (int)strtol(line[1], &error, 10);
            if (*error != '\0') {
                width = -1;
            }
        } else {
            if (game->isXTurn) {
                get_auto_move_for_x(&height, &width, game);
            } else {
                get_auto_move_for_o(&height, &width, game);
            }
        }
    } while (!is_move_valid(height, width, game));
    game->board[height][width] = player->playerName;
    if (!player->isManual) {
        printf("Player %c => %d %d\n", player->playerName, height, width);
    }
    return check_game_over(height, width, player->playerName, game);
}

/**
    Starts the game and returns 0 upon normal completion of the game
**/
int start_game(Game* game) {
    bool isGameOver = false;
    while (!isGameOver) {
        if (game->isXTurn) {
            isGameOver = get_move(game->players[1], game);
        } else {
            isGameOver = get_move(game->players[0], game);
        }
        game->isXTurn = !game->isXTurn;
        print_game(game);
    }
    printf("Player %c wins\n", game->winner);
    free_game(game);
    return 0;
}

/**
    The main function of the program
**/
int main(int argc, char** argv) {
    if ((argc != 4) && (argc != 5)) {
        return show_error_message(USAGE);
    }
    if ((strlen(argv[1]) != 1) || (strlen(argv[2]) != 1)) {
        return show_error_message(PLAYER_TYPE);
    }
    if (((argv[1][0] != 'm') && (argv[1][0] != 'a')) || 
            ((argv[2][0] != 'm') && (argv[2][0] != 'a'))) {
        return show_error_message(PLAYER_TYPE);
    }
    int height, width;
    Game* game = NULL;
    if (argc == 5) {
        char* dimensionsError = 0;
        height = (int)strtol(argv[3], &dimensionsError, 10);
        if (*dimensionsError != '\0' || height <= 0 || height > 1000) {
            return show_error_message(GRID_DIMENSIONS);
        }
        width = (int)strtol(argv[4], &dimensionsError, 10);
        if (*dimensionsError != '\0' || width <= 0 || width > 1000) {
            return show_error_message(GRID_DIMENSIONS);
        }
        game = initialize_game(height, width);
        game->isXTurn = false;
    } else {
        FILE* gameFile = fopen(argv[3], "r");
        if (!gameFile) {
            return show_error_message(FILE_READ);
        }
        if (load_game(gameFile, &game) < 0) {
            return show_error_message(INVALID_FILE);
        }
    }
    initialize_player(argv[1], game->players[0], 0);
    initialize_player(argv[2], game->players[1], 0);
    print_game(game);

    return start_game(game);
}

/**
    Frees the game resources
**/
void free_game(Game* game) {
    if (game != 0) {
        for (int i = 0; i < game->height; i++) {
            free(game->board[i]);
        }
        free(game->board);
        free(game->players[0]);
        free(game->players[1]);
        free(game);
    }
}

/**
    Frees the stack resources
**/
void free_stack(Stack* stack) {
    free(stack->rows);
    free(stack->columns);
    free(stack->rowLog);
    free(stack->columnLog);
    free(stack);
}

/**
    Splits a given string using the delimitter provided, and returns
    an array of null terminated strings.
    @param tokenCount sets the value pointed by it to the number of
    	tokens the splitted string contains.
**/
char** split_string(char* line, int* tokenCount, char* delimitter) {
    char** splitString = malloc(sizeof(char*) * 50);
    for (int i = 0; i < 50; i++) {
        splitString[i] = malloc(sizeof(char) * 50);
    }
    int index = 0;
    splitString[index++] = strtok(line, delimitter);
    (*tokenCount)++;
    while ((splitString[index++] = strtok(NULL, delimitter)) != NULL) {
        (*tokenCount)++;
    }
    return splitString;
}
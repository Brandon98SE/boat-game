/*
CHANGELOG (2025-02-11):
    - Fixed user input error handling.
    - Added text passthrough to input_int().
    - Created Main Menu.
    - next steps: Start on coding task_2.

TASK_1:
    output = {1. New Game, 2. Resume Game, 3. Instructions, 4. Quit}
    input = answer()
    output(PLayer 2 name: "{input}", Computer Name: {random_name})
    input(name, comp)
    END

TASK_2:
    - DONE: Display blank grid.
    - ask coords from user
    - check conflicting
    - display boats on the grid
    - made the grid - made column and rows.
    - made menu system more robust
    - better error handling.
    - WATTTTER!!!
    - made more modular
    FOR TMR:
        - Render boats on board.
        - allow to choose where to render boat, w/ error handing
        - tidy up code.
*/

// libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// global macros
#define BOARD_SIZE_MAX 26
#define ERROR_LINES 100
#define MAX_BOATS 5
#define STD_ERR "Invalid option!"
#define MAX_CHARS 3
#define ICON_HIT 'X'
#define ICON_MISS '~'

// function prototypes
void clrscr(int i);
void render_board(int board_size);
void main_menu(void);
void new_game(int board_size);
void reset_board(int board_size);
int input_int(char *text, char *error, int clrscr_lines, int error_lines, int after_lines, int lower_bound, int upper_bound);
void input_coordinates(int board_size);
void computer_coordinates(int board_size);
int input_option(char* message);
int check_hit(char *pos_temp);
void player_attack(int board_size);

// global constants
const char TEST_BOARD[BOARD_SIZE_MAX][BOARD_SIZE_MAX] = {
    {'O', '~', 'O'},
    {'~', 'X', '~'},
    {'O', 'X', 'O'}
};

// global variables
char board[BOARD_SIZE_MAX][BOARD_SIZE_MAX];
char pos_final[MAX_BOATS][MAX_CHARS];
char comp_final[MAX_BOATS][MAX_CHARS];

// main program
int main(void)
{
    clrscr(ERROR_LINES);
    int menu_selection;
    while (1)
    {
        menu_selection = input_int("--- GAME MENU ---\n1. New Game\n2. Resume Game\n3. Instructions\n0. Quit\n", "ERROR: Invalid Option.\n", 0, ERROR_LINES, 0, 0, 3);
        if (menu_selection == 0) // quit game
        {
            clrscr(ERROR_LINES);
            exit(0);
        }
        else if (menu_selection == 1) // New Game
        {
            // choose board size
            clrscr(ERROR_LINES);
            int board_size = input_int("--- NEW GAME ---\nSize of board (3 to 26): ", "ERROR: Invalid Option.\n", 0, ERROR_LINES, 100, 3, 26);

            // RENDER PLAYER BOARD FOR DEBUG
            reset_board(board_size);
            render_board(board_size); // ENABLED so the user can view the board to see valid locations
            input_coordinates(board_size);
            for (int i = 0; (i < MAX_BOATS); i++)
            {
                board[(int) pos_final[i][0] - 'A'][(int) pos_final[i][1] - '1'] = '.';
            }
            printf("DEBUG: PLAYER BOARD.\n");
            render_board(board_size); // only enabled for DEBUGGING!

            // RENDER COMPUTER BOARD FOR DEBUG
            reset_board(board_size);
            computer_coordinates(board_size);
            for (int i = 0; (i < MAX_BOATS); i++)
            {
                board[(int) comp_final[i][0] - 'A'][(int) comp_final[i][1] - '1'] = '.';
            }
            printf("DEBUG: COMPUTER BOARD.\n");
            render_board(board_size); // only enabled for DEBUGGING!

            // ATTACKING:
            reset_board(board_size);
            for (int i = 0; i < 999; i++)
            {
                player_attack(board_size);
                render_board(board_size);
            }
        }
        else if (menu_selection == 2 || menu_selection == 3) // Resume game
        {
            clrscr(ERROR_LINES);
            printf("ERROR: This mode has not been implemented yet.\n\n");
        }
        else // error handling
        {
            clrscr(ERROR_LINES);
            printf("ERROR: Impossible menu option selected - THIS IS A BUG.\n\n");
            exit(1);
        }
    }
}

void player_attack(int board_size)
{
    char pos_temp[MAX_CHARS]; // [x] [y] [\0]
    static int hit_counter = 0;
    static int player_tries = 0;

    if (hit_counter >= MAX_BOATS)
    {
        printf("Debug Temporary Message: YOU ARE THE WINNER!!\n");
        exit(0);
    }

    while (1)
    {
        // ✅ input coords
        printf("Enter coordinates to attack (e.g: A2): ");
        scanf("%2s", pos_temp);

        printf("DEBUG: PLAYER_TRIES: %i", player_tries);
        // ✅ check if valid
        if (pos_temp[0] >= 'a' && pos_temp[0] <= 'z') {pos_temp[0] -= 32;}
        if ((pos_temp[0] >= 'A' && pos_temp[0] <= (board_size + 'A' - 1)) &&
        (pos_temp[1] >= '1' && pos_temp[1] <= (board_size + '0')) &&
        (pos_temp[2] == '\0'))
        {
            // convert to int
            int board_row = pos_temp[0] - 'A';
            int board_collum = pos_temp[1] - '1';

            // check to see if area used
            if (board[board_row][board_collum] == ICON_HIT || board[board_row][board_collum] == ICON_MISS)
            {
                printf("DEBUG: You are attacking the same position twice!\n");
            }
            else
            {
                hit_counter++;
            }
            int hit = check_hit(pos_temp);
            board[board_row][board_collum] = (hit) ? ICON_HIT : ICON_MISS;
        }
        else
        {
            printf("ERROR: Invalid Coordinate, please re-enter!\n");
        }
        break;
    }
}

int check_hit(char *pos_temp)
{
    for (int i = 0; i < MAX_BOATS; i++)
    {
        if (strcmp(comp_final[i], pos_temp) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void computer_coordinates(int board_size)
{
    static char pos_temp[MAX_CHARS]; // [x] [y] [\0]
    srand(time(NULL));
    for (int l = 0; l < MAX_BOATS; l++)
    {
        // coordinate checking
        while (1)
        {
            // Generate random position
            pos_temp[0] = 'A' + (rand() % board_size);
            pos_temp[1] = '1' + (rand() % board_size);
            pos_temp[2] = '\0';

            // check for conflicting coords
            int coordinate_conflict = 0;
            for (int i = 0; i < l; i++)
            {
                if (strcmp(comp_final[i], pos_temp) == 0)
                {
                    coordinate_conflict = 1;
                    break;
                }
            }
            if (!coordinate_conflict)
            {
                break;
            }
        }
        strcpy(comp_final[l], pos_temp);
    }
}

int input_option(char* message)
{
    char option[2]; // [y/n] [\0]
    while (1)
    {
        // input coords
        printf("%s", message);
        scanf("%1s", option);

        // checks if valid
        if (option[0] == 'Y' || option[0] == 'y')
        {
            return 1;
        }
        else if (option[0] == 'N' || option[0] == 'n')
        {
            return 0;
        }
        printf("%s\n", STD_ERR);
    }
}

void input_coordinates(int board_size)
{
    static char pos_temp[MAX_CHARS]; // [x] [y] [\0]
    // storing.
    //    pos_final[3 char per boat][5 boats]
    for (int l = 0; l < MAX_BOATS; l++)
    {
        // coordinate checking
        while (1)
        {
            // ✅ input coords
            printf("Enter your boat coordinates (e.g: A2): ");
            scanf("%2s", pos_temp);

            // ✅ convert to upper
            if (pos_temp[0] >= 'a' && pos_temp[0] <= 'z')
            {
                pos_temp[0] -= 32;
            }

            // ✅ check if valid
            if ((pos_temp[0] >= 'A' && pos_temp[0] <= (board_size + 'A' - 1)) &&
            (pos_temp[1] >= '1' && pos_temp[1] <= (board_size + '0')) &&
            (pos_temp[2] == '\0'))
            {
                // ❌
                int coordinate_conflict = 0;
                for (int i = 0; i < l; i++)
                {
                    if (strcmp(pos_final[i], pos_temp) == 0)
                    {
                        coordinate_conflict = 1;
                        break;
                    }
                }
                if (!coordinate_conflict)
                {
                    break;
                }
                printf("ERROR: Coordinate already used, please re-enter!\n");
            }
            else
            {
                printf("ERROR: Invalid Coordinate, please re-enter!\n");
            }
        }
        // pos_final pos_temp as used_coordinates[MAX_BOATS] (5) = A2, A3, B5, A5, D3. compare them with input, if == INVALID.
        strcpy(pos_final[l], pos_temp);
    }
}

void reset_board(int board_size)
{
    for (int i = 0; i < board_size; i++)
    {
        for (int j = 0; j < board_size; j++)
        {
            board[i][j] = '_';
        }
    }
}

void render_board(int board_size)
{
    //render characters at top
    printf("   ");
    for (int i = 0; i < board_size; i++)
    {
            printf("_%c_ ", (char) i + 65);
    }
    //renders numbers at left side
    for (int i = 0; i < board_size; i++)
    {
        printf("\n%2i", i + 1);
        //renders game grid
        for (int j = 0; j < board_size; j++)
        {
            printf("|_%c_", board[j][i]);
        }
        printf("|");
    }
    printf("\n");
}

int input_int(char *text, char *error, int clrscr_lines, int error_lines, int after_lines, int lower_bound, int upper_bound)
{
    int input = 0, result = 0, character = 0;
    if (clrscr_lines != 0)
    {
        clrscr(clrscr_lines);
    }
    do
    {
        printf("%s", text);
        result = scanf("%d", &input);
        if (result == 0 || input < lower_bound || input > upper_bound)
        {
            for (int i = 0; i < error_lines; i++)
            {
                printf("\n");
            }
            printf("%s\n", error);
            while ((character = getchar()) != '\n' && character != EOF);
            result = 0;
        }
    }
    while (result == 0);
    if (after_lines != 0)
    {
        clrscr(after_lines);
    }
    return input;
}

void clrscr(int i)
{
    for (int n = 0; n < i; n++)
    {
        printf("\n");
    }
}

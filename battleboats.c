// libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// global macros
#define VER "Battleboats v0.1.6-alpha"
#define BOARD_SIZE_MAX 9
#define ERROR_LINES 100
#define MAX_BOATS 5
#define STD_ERR "Invalid option!"
#define MAX_CHARS 3
#define ICON_HIT 'X'
#define ICON_MISS '~'

#ifdef _WIN32
    #include <windows.h> // windows
    #define SLEEP(ms) Sleep(ms) // sleep on windows is in ms.
#else
    #include <unistd.h> // linux/macos
    #define SLEEP(ms) usleep((ms) * 1000) // sleep is in micro seconds.
#endif

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
void instructions(void);
void splash_screen(void);

// global variables
char board[BOARD_SIZE_MAX][BOARD_SIZE_MAX];
char pos_final[MAX_BOATS][MAX_CHARS];
char comp_final[MAX_BOATS][MAX_CHARS];

// main program
int main(void)
{
    splash_screen();
    clrscr(ERROR_LINES);
    int menu_selection;
    while (1)
    {
        menu_selection = input_int("--- GAME MENU ---\n1. New Game\n2. Resume Game\n3. Instructions\n0. Quit\n", "ERROR: Invalid Option.\n", 0, ERROR_LINES, 0, 0, 3);
        if (menu_selection == 0) // quit game
        {
            clrscr(ERROR_LINES);
            printf("--- %s ---\nThank you for playing!\n", VER);
            exit(0);
        }
        else if (menu_selection == 1) // New Game
        {
            // choose board size
            clrscr(ERROR_LINES);
            int board_size = input_int("--- NEW GAME ---\nSize of board (3 to 9): ", "ERROR: Invalid Option.\n", 0, ERROR_LINES, 100, 3, 9);

            // RENDER PLAYER BOARD FOR DEBUG
            reset_board(board_size);
            input_coordinates(board_size);
            printf("DEBUG: PLAYER BOARD.\n");
            render_board(board_size); // only enabled for DEBUGGING!

            // RENDER COMPUTER BOARD FOR DEBUG
            reset_board(board_size);
            computer_coordinates(board_size);
            for (int i = 0; (i < MAX_BOATS); i++)
            {
                board[(int) comp_final[i][0] - 'A'][(int) comp_final[i][1] - '1'] = 'C';
            }
            printf("DEBUG: COMPUTER BOARD.\n");
            render_board(board_size); // only enabled for DEBUGGING!

            // ATTACKING:
            player_attack(board_size);
        }
        else if (menu_selection == 2) // Resume game
        {
            clrscr(ERROR_LINES);
            printf("ERROR: This mode has not been implemented yet.\n\n");
        }
        else if (menu_selection == 3) // Instructions
        {
            instructions();
            menu_selection = 0;
        }
        else // error handling
        {
            clrscr(ERROR_LINES);
            printf("ERROR: Impossible menu option selected - THIS IS A BUG.\n\n");
            exit(1);
        }
    }
}

void splash_screen(void)
{
    clrscr(100);
    printf("--- %s ---\n", VER);
    fflush(stdout);
    SLEEP(2000); // sleep for 2 seconds
}

void instructions(void)
{
    int submenu = 0;
    int menu_option = 0;
    while (1)
    {
        int menu_option = input_int("--- Instruction Categories ---\n\n1. How to play Battleboats\n2. How to win\n3. Controls\n0. RETURN TO MAIN MENU\n", "INVALID OPTION, please re-enter!\n", 100, 100, 100, 0, 3);
        if (!menu_option)
        {
            break;
        }
        else if (menu_option == 1)
        {
            submenu = input_int("--- How to play Battleboats ---\n\nSelect 5 coordinates for your boats\nthen you take in turns to hit\nyour opponent's boats\nthen you win after killing all\n5 of their boats!\n\n0. Go back\n", STD_ERR, 100, 100, 100, 0, 0);
        }
        else if (menu_option == 2)
        {
            submenu = input_int("--- How to win ---\n\nSelect the boats that the\ncomputer has chosen, you\nhave to sink the computer's\nboats to win the game\nGood Luck! :D\n\n0. Go back\n", STD_ERR, 100, 100, 100, 0, 0);
        }
        else if (menu_option == 3)
        {
            submenu = input_int("--- Controls ---\n\nUse the letters and numbers\non your keyboard and press return\nto submit your choices.\n\n0. Go back\n", STD_ERR, 100, 100, 100, 0, 0);
        }
        else
        {
            printf("Invalid Option, program HALTED!\n");
            exit(6);
        }
    }
}

void player_attack(int board_size)
{
    char pos_temp[MAX_CHARS]; // [x] [y] [\0]
    static int hit_counter = 0;
    static int hit = 0;

    while (1) // for error handling
    {
        if (hit_counter == MAX_BOATS)
        {
            printf("VICTORY: YOU ARE THE WINNER!!\n");
            exit(0);
        }
        while (1)
        {
            // input coords
            printf("Enter coordinates to attack (e.g: A2): ");
            scanf("%3s", pos_temp);

            // converting output to capital
            if (pos_temp[0] >= 'a' && pos_temp[0] <= 'z') {pos_temp[0] -= 32;}

            // checking if valid
            if ((pos_temp[0] >= 'A' && pos_temp[0] <= (board_size + 'A' - 1)) &&
            (pos_temp[1] >= '1' && pos_temp[1] <= (board_size + '0')) &&
            (pos_temp[2] == '\0'))
            {
                // convert to int from char
                int x = pos_temp[0] - 'A';
                int y = pos_temp[1] - '1';

                // check to see if area used 
                if (board[x][y] == ICON_HIT || board[x][y] == ICON_MISS) // hit same part twice: "X" or "~"
                {
                    clrscr(100);
                    render_board(board_size);
                    printf("You are attacking the same position twice!\n");
                    continue;
                }

                // check if hit any enemy boats: 'U', 'C'
                for (int i = 0; i < MAX_BOATS; i++)
                {
                    clrscr(100);
                    if (strcmp(comp_final[i], pos_temp) == 0)
                    {
                        hit = 1;
                        board[x][y] = hit ? ICON_HIT : ICON_MISS; // update board
                        hit_counter++;
                        render_board(board_size);
                        printf("You destroyed an enemy boat!\n");
                        break;
                    }
                }

                // if hit water
                if (!hit)
                {
                    clrscr(100);
                    board[x][y] = hit ? ICON_HIT : ICON_MISS; // update board
                    render_board(board_size);
                    printf("You hit the water!\n");
                    // nothing happens
                }

                // reset hit
                hit = 0;

                break;
            }
            else
            {
                printf("ERROR: Invalid Coordinate, please re-enter!\n");
            }
        }
    }
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
            clrscr(100);
            render_board(board_size); // ENABLED so the user can view the board to see valid locations
            // ✅ input coords
            printf("Enter your boat coordinates (e.g: A2): ");
            scanf("%3s", pos_temp);

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
                clrscr(100);
                printf("ERROR: Coordinate already used, please re-enter!\n");
            }
            else
            {
                clrscr(100);
                printf("ERROR: Invalid Coordinate, please re-enter!\n");
            }
        }
        // pos_final pos_temp as used_coordinates[MAX_BOATS] (5) = A2, A3, B5, A5, D3. compare them with input, if == INVALID.
        strcpy(pos_final[l], pos_temp);
        for (int i = 0; (i <= l); i++)
        {
            board[(int) pos_final[i][0] - 'A'][(int) pos_final[i][1] - '1'] = 'U';
        }
    }
    clrscr(100);
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

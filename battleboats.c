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
#define CLRSCR 0
#define BOARD_COMP_ATTACK 2
#define BOARD_COMP 1
#define BOARD_USER 0
#define BOARD_USER_ATTACK 3

#ifdef _WIN32
    #include <windows.h> // windows
    #define SLEEP(ms) Sleep(ms) // sleep on windows is in ms.
#else
    #include <unistd.h> // linux/macos
    #define SLEEP(ms) usleep((ms) * 1000) // sleep is in micro seconds.
#endif

// function prototypes
void clrscr(int i);
void render_board(int board_size, int option);
void main_menu(void);
void new_game(int board_size);
void reset_board(int board_size, int option);
int input_int(char *text, char *error, int clrscr_lines, int error_lines, int after_lines, int lower_bound, int upper_bound);
void input_coordinates(int board_size);
void computer_coordinates(int board_size);
int input_option(char* message);
int check_hit(char *pos_temp);
void player_attack(int board_size);
void instructions(void);
void splash_screen(void);
void computer_attack(int board_size);
void save_game(void);
void resume_game(void);

// global variables
char board_user[BOARD_SIZE_MAX][BOARD_SIZE_MAX];
char board_comp[BOARD_SIZE_MAX][BOARD_SIZE_MAX];
char board_comp_attack[BOARD_SIZE_MAX][BOARD_SIZE_MAX];
char board_user_attack[BOARD_SIZE_MAX][BOARD_SIZE_MAX];
char pos_final[MAX_BOATS][MAX_CHARS];
char comp_final[MAX_BOATS][MAX_CHARS];
static int user_hit_counter = 0;
static int comp_hit_counter = 0;
char save_code[1000];
int user_boat_cells[BOARD_SIZE_MAX][BOARD_SIZE_MAX]; // track player boat pos
int comp_boat_cells[BOARD_SIZE_MAX][BOARD_SIZE_MAX]; // track comp boat pos
int user_boat_hits[5] = {0}; // track per boat hit in user_boat_cells
int comp_boat_hits[5] = {0}; // track per boat hit in user_boat_cells
const int user_boat_sizes[5] = {1, 1, 2, 2, 3}; // 2x destrpyser, 2x other thing, 1x carrier
const int comp_boat_sizes[5] = {1, 1, 2, 2, 3}; // 2x destrpyser, 2x other thing, 1x carrier

int board_size = 0;


// main program
int main(void)
{
    srand(time(NULL)); // random seed.
    splash_screen();
    SLEEP(1500);
    clrscr(ERROR_LINES);
    int menu_selection;
    while (1)
    {
        menu_selection = input_int("--- GAME MENU ---\n1. New Game\n2. Resume Game\n3. Instructions\n0. Quit\n", "ERROR: Invalid Option.\n", 0, ERROR_LINES, 0, 0, 3);
        if (menu_selection == 0) // quit game
        {
            clrscr(ERROR_LINES);
            printf("--- %s ---\nThank you for playing!\n", VER);
            printf("Press any key to exit.\n");
            getchar();
            exit(0);
        }
        else if (menu_selection == 1) // New Game
        {
            // choose board size
            clrscr(ERROR_LINES);
            board_size = input_int("--- NEW GAME ---\nSize of board (3 to 9): ", "ERROR: Invalid Option.\n", 0, ERROR_LINES, 100, 3, 9);

            // RENDER PLAYER BOARD FOR DEBUG
            reset_board(board_size, BOARD_USER);
            input_coordinates(board_size);
            //printf("DEBUG: PLAYER BOARD.\n");
            //render_board(board_size, BOARD_USER); // only enabled for DEBUGGING!

            // RENDER COMPUTER BOARD FOR DEBUG
            reset_board(board_size, BOARD_COMP);
            computer_coordinates(board_size);
            for (int i = 0; (i < MAX_BOATS); i++)
            {
                board_comp[(int) comp_final[i][0] - 'A'][(int) comp_final[i][1] - '1'] = 'C';
            }
            //printf("DEBUG: COMPUTER BOARD.\n");
            //render_board(board_size, BOARD_COMP); // only enabled for DEBUGGING!

            // ATTACKING:
            reset_board(board_size, BOARD_COMP_ATTACK);
            reset_board(board_size, BOARD_USER_ATTACK);
            printf("The computer is choosing boat positions...\n");
            SLEEP(1500);
            clrscr(100);
            printf("--- Player hit tracker ---\n");
            render_board(board_size, BOARD_COMP_ATTACK);
            while (comp_hit_counter < MAX_BOATS && user_hit_counter < MAX_BOATS)
            {
                player_attack(board_size);
                SLEEP(3000);
                computer_attack(board_size);
            }
        }
        else if (menu_selection == 2) // Resume game
        {
            clrscr(100);
            resume_game();
            //printf("DEBUG: COMPUTER BOARD.\n");

            // Check to see if game has ended already
            if (user_hit_counter >= MAX_BOATS || comp_hit_counter >= MAX_BOATS) 
            {
                printf("The resumed game has already finished!\n");
                SLEEP(2000);
                continue;
            }

            printf("Resuming game...\n");
            SLEEP(1500);
            clrscr(100);
            printf("--- Player hit tracker ---\n");
            render_board(board_size, BOARD_COMP_ATTACK);
            //render_board(board_size, BOARD_USER);

            // run resumed game
            while (comp_hit_counter < MAX_BOATS && user_hit_counter < MAX_BOATS)
            {
                player_attack(board_size);
                SLEEP(3000);
                computer_attack(board_size);
            }

            // Check if win/lose
            if (user_hit_counter >= MAX_BOATS) 
            {
                splash_screen();
                printf("VICTORY: YOU ARE THE WINNER!!\n");
            } else if (comp_hit_counter >= MAX_BOATS) 
            {
                splash_screen();
                printf("DEFEAT: YOU HAVE LOST! >:C\n");
            }

            // end
            printf("Press any key to exit.\n");
            getchar();
            exit(0);
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

void resume_game(void)
{
    char input_code[1000];
    printf("Paste your resume code here (CTRL+SHIFT+V) and press ENTER: ");
    scanf("%999s", input_code);

    int count = 0;

    // board_size
    if (input_code[count] < '3' || input_code[count] > '9') 
    {
        printf("ERROR: Invalid save code, board_size out of range!\n");
        return;
    }
    board_size = input_code[count++] - '0';

    // board_user
    for (int i = 0; i < board_size; i++) 
    {
        for (int j = 0; j < board_size; j++) 
        {
            if (input_code[count] == '\0') 
            {
                printf("ERROR: Invalid save code, board_user data has been corrupted!\n");
                return;
            }
            board_user[i][j] = input_code[count++];
        }
    }

    // board_comp
    for (int i = 0; i < board_size; i++) 
    {
        for (int j = 0; j < board_size; j++) 
        {
            if (input_code[count] == '\0') 
            {
                printf("ERROR: Invalid save code, board_comp data has been corrupted!\n");
                return;
            }
            board_comp[i][j] = input_code[count++];
        }
    }

    // board_comp_attack
    for (int i = 0; i < board_size; i++) 
    {
        for (int j = 0; j < board_size; j++) 
        {
            if (input_code[count] == '\0') 
            {
                printf("ERROR: Invalid save code, board_comp_attack data has been corrupted!\n");
                return;
            }
            board_comp_attack[i][j] = input_code[count++];
        }
    }

    // board_user_attack
    for (int i = 0; i < board_size; i++) 
    {
        for (int j = 0; j < board_size; j++) 
        {
            if (input_code[count] == '\0') 
            {
                printf("ERROR: Invalid save code, board_user_attack data has been corrupted!\n");
                return;
            }
            board_user_attack[i][j] = input_code[count++];
        }
    }

    // pos_final
    for (int i = 0; i < MAX_BOATS; i++) 
    {
        if (input_code[count] == '\0' || input_code[count + 1] == '\0') 
        {
            printf("ERROR: Invalid save code, pos_final data has been corrupted!\n");
            return;
        }
        pos_final[i][0] = input_code[count++];
        pos_final[i][1] = input_code[count++];
        pos_final[i][2] = '\0';
    }

    // comp_final
    for (int i = 0; i < MAX_BOATS; i++) 
    {
        if (input_code[count] == '\0' || input_code[count + 1] == '\0') 
        {
            printf("ERROR: Invalid save code, comp_final data has been corrupted!\n");
            return;
        }
        comp_final[i][0] = input_code[count++];
        comp_final[i][1] = input_code[count++];
        comp_final[i][2] = '\0';
    }

    // user_hit_counter
    if (count + 3 > strlen(input_code)) 
    {
        printf("ERROR: Invalid save code, missing hit counters.\n");
        return;
    }
    char user_hits[4] = {input_code[count], input_code[count+1], input_code[count+2], '\0'};
    user_hit_counter = atoi(user_hits);
    count += 3;

    // comp_hit_counter
    char comp_hits[4] = {input_code[count], input_code[count+1], input_code[count+2], '\0'};
    comp_hit_counter = atoi(comp_hits);
    count += 3;

    printf("Game resumed!\n");
}

/*
Varaiables that need saving:
+ board_user
+ board_comp
+ board_comp_attack
+ board_user_attack
+ pos_final
+ comp_final
+ user_hit_counter
+ comp_hit_counter
+ board_size
*/

void save_game()
{
    int count = 0;

    // board_size
    save_code[count++] = '0' + board_size;

    // board_user
    for (int i = 0; i < board_size; i++) 
    {
        for (int j = 0; j < board_size; j++) 
        {
            save_code[count++] = board_user[i][j];
        }
    }

    // board_comp
    for (int i = 0; i < board_size; i++) 
    {
        for (int j = 0; j < board_size; j++) 
        {
            save_code[count++] = board_comp[i][j];
        }
    }

    // board_comp_attack
    for (int i = 0; i < board_size; i++) 
    {
        for (int j = 0; j < board_size; j++) 
        {
            save_code[count++] = board_comp_attack[i][j];
        }
    }

    // board_user_attack
    for (int i = 0; i < board_size; i++) 
    {
        for (int j = 0; j < board_size; j++) 
        {
            save_code[count++]= board_user_attack[i][j];
        }
    }

    // pos_final /wo null terminatror 
    for (int i = 0; i < MAX_BOATS; i++) 
    {
        save_code[count++] = pos_final[i][0];
        save_code[count++] = pos_final[i][1];
    }

    // comp_final /wo null terminator
    for (int i = 0; i < MAX_BOATS; i++) 
    {
        save_code[count++] = comp_final[i][0];
        save_code[count++] = comp_final[i][1];
    }

    // user_hit_counter str
    sprintf(&save_code[count], "%03d", user_hit_counter);
    count += 3;

    // comp_hit_counter str
    sprintf(&save_code[count], "%03d", comp_hit_counter);
    count +=3;

    // null terminator
    save_code[count] = '\0';

    printf("Your save code is: %s\n", save_code);
}

void splash_screen(void)
{
    clrscr(100);
    printf("--- %s ---\n", VER);
    //SLEEP(2000); // sleep for 2 seconds
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
    char pos_temp[5]; // [x] [y] [\0]
    static int hit = 0;
    while (1)
    {
        do
        {
            // input coords
            printf("Enter coordinates to attack (e.g: \"A2\", or \"save\" and exit, or \"exit\" without saving.): ");
            scanf("%5s", pos_temp);
            if (strcmp(pos_temp, "save") == 0)
            {
                save_game();
                printf("Saved and exited, please take a copy your save code, thank you for playing battleboats!\n");
                exit(0);
            }
            else if (strcmp(pos_temp, "exit") == 0)
            {
                printf("Exited without saving, thank you for playing battleboats!\n");
                exit(0);
            }
        }
        while (strcmp(pos_temp, "save") == 0 || strcmp(pos_temp, "exit") == 0);

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
            if (board_comp_attack[x][y] == ICON_HIT || board_comp_attack[x][y] == ICON_MISS) // hit same part twice: "X" or "~"
            {
                clrscr(100);
                printf("--- Player hit tracker ---\n");
                render_board(board_size, BOARD_COMP_ATTACK);
                printf("You are attacking the same position twice at %s!\n", pos_temp);
                continue;
            }

            // check if hit any enemy boats: 'U', 'C'
            hit = 0;
            for (int i = 0; i < MAX_BOATS; i++)
            {
                clrscr(100);
                printf("--- Player hit tracker ---\n");
                if (strcmp(comp_final[i], pos_temp) == 0)
                {
                    hit = 1;
                    board_comp_attack[x][y] = hit ? ICON_HIT : ICON_MISS; // update board
                    user_hit_counter++;
                    render_board(board_size, BOARD_COMP_ATTACK);
                    printf("You destroyed an enemy boat at %s!\n", pos_temp);
                    break;
                }
            }

            // if hit water
            if (!hit)
            {
                clrscr(100);
                printf("--- Player hit tracker ---\n");
                board_comp_attack[x][y] = hit ? ICON_HIT : ICON_MISS; // update board
                render_board(board_size, BOARD_COMP_ATTACK);
                printf("You hit the water at %s!\n", pos_temp);
                // nothing happens
            }

            break;
        }
        else
        {
            printf("ERROR: Invalid coordinate or option, please re-enter!\n");
        }
    }
    if (user_hit_counter == MAX_BOATS)
    {
        splash_screen();
        printf("VICTORY: YOU ARE THE WINNER!!\n");
        printf("Press any key to exit.\n");
        getchar();
        exit(0);
    }
    printf("\n");
}

void computer_attack(int board_size)
{
    char comp_temp[MAX_CHARS]; // [x] [y] [\0]
    static int hit = 0;
    int x = 0;
    int y = 0;

    printf("--- Computer hit tracker ---\n");

    while (1)
    {
        // Generate random position
        comp_temp[0] = 'A' + (rand() % board_size);
        comp_temp[1] = '1' + (rand() % board_size);
        comp_temp[2] = '\0';

        // convert to int from char
        int x = comp_temp[0] - 'A';
        int y = comp_temp[1] - '1';

        // check to see if area used 
        if (board_user_attack[x][y] == ICON_HIT || board_user_attack[x][y] == ICON_MISS) // hit same part twice: "X" or "~"
        {
            //render_board(board_size, BOARD_USER_ATTACK);
            //printf("The computer is attacking the same position twice at %s!\n", comp_temp);
            continue;
        }

        // check if computer hit any of our boats: 'U', 'C'
        hit = 0;
        for (int i = 0; i < MAX_BOATS; i++)
        {
            if (strcmp(pos_final[i], comp_temp) == 0) // 'A' '3' '\0'.
            {
                hit = 1;
                board_user_attack[x][y] = hit ? ICON_HIT : ICON_MISS; // update board
                comp_hit_counter++;
                render_board(board_size, BOARD_USER_ATTACK);
                printf("The computer destroyed one of your boats at %s!\n\n", comp_temp);
                if (comp_hit_counter == MAX_BOATS)
                {
                    splash_screen();
                    printf("DEFEAT: YOU HAVE LOST! >:C\n");
                    printf("Press any key to exit.\n");
                    getchar();
                    exit(0);
                }
                return;
            }
        }

        // computer hit water
        if (!hit)
        {
            board_user_attack[x][y] = ICON_MISS; // update board
            render_board(board_size, BOARD_USER_ATTACK);
            printf("The computer hit water at %s!\n\n", comp_temp);
            return;
        }
    }
}

void computer_coordinates(int board_size)
{
    char pos_temp[MAX_CHARS]; // [x] [y] [\0]
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
                strcpy(comp_final[l], pos_temp);
                break;
            }
        }
    }
    // //printf("Final coords: ");
    // for (int i = 0; i < MAX_BOATS; i++)
    // {
    //     printf("%s, ", comp_final[i]);
    // }
    // printf("\n");
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
    char pos_temp[MAX_CHARS]; // [x] [y] [\0]
    char orientation;
    // input boats in order of size, 5 boats with 3 variatiosn.
    int boat_order[5] = {1, 1, 2, 2, 3};
    // storing.
    //    pos_final[3 char per boat][5 boats]
    for (int l = 0; l < MAX_BOATS; l++)
    {
        // size of current boat
        int size = boat_order[l];
        clrscr(100);
        // coordinate checking
        while (1)
        {
            render_board(board_size, BOARD_USER); // ENABLED so the user can view the board to see valid locations
            // ✅ input coords
            printf("Enter boat %d (Size %d), coordinates (e.g: A2): ", l+1, size);
            scanf("%3s", pos_temp);

            // ✅ convert to upper
            if (pos_temp[0] >= 'a' && pos_temp[0] <= 'z')
            {
                pos_temp[0] -= 32;
            }

            if (!pos_valid(pos_temp, board_size))
            {
                printf("ERROR: Invalid Coordinates, please re-enter!\n");
                continue;
            }

            if (size > 1)
            {
                printf("Enter boat orientation: ");
                scanf("%c", &orientation);
                // ✅ convert to upper
                if (orientation[0] >= 'a' && orientation[0] <= 'z')
                {
                    orientation[0] -= 32;
                }
                if (orientation != 'H' && orientation != 'V')
                {
                    printf("ERROR: Invalid Orientation, please re-enter!\n");
                    continue;
                }
            }

            // boat pos
            char cells[3][3];
            if (!boat_cells(pos_temp, orientation, size, board_size, cells))
            {
                printf("ERROR: Boat out of bounds, please re-enter!\n");
                continue;
            }

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
        }
        // pos_final pos_temp as used_coordinates[MAX_BOATS] (5) = A2, A3, B5, A5, D3. compare them with input, if == INVALID.
        strcpy(pos_final[l], pos_temp);
        for (int i = 0; (i <= l); i++)
        {
            board_user[(int) pos_final[i][0] - 'A'][(int) pos_final[i][1] - '1'] = 'U';
        }
    }
    clrscr(100);
}

bool boat_cells(char *start_coords, char orientation, int size, int board_size, char cells[3])
{
    int x_coord = start_coords[0] - 'A';
    int y_coord = start_coords[1] - '1';

    for (int i = 0; i < size; i++)
    {
        int x = x_coord + orientation == 'V' ? i : 0); // H add to right, V add to bottom
        int y = y_coord + orientation == 'H' ? i : 0); // H add to right, V add to bottom
    }
}

int pos_valid(char *pos, int board_size)
{
    // ✅ check if valid
    return (pos[0] >= 'A' && pos[0] <= (board_size + 'A' - 1)) &&
            (pos[1] >= '1' && pos[1] <= (board_size + '0')) &&
            (pos[2] == '\0');
}


void reset_board(int board_size, int option)
{
    for (int i = 0; i < board_size; i++)
    {
        for (int j = 0; j < board_size; j++)
        {
            if (option == BOARD_COMP)
            {
                board_comp[i][j] = '_';
            }
            else if (option == BOARD_USER)
            {
                board_user[i][j] = '_';
            }
            else if (option == BOARD_COMP_ATTACK)
            {
                board_comp_attack[i][j] = '_';
            }
            else if (option == BOARD_USER_ATTACK)
            {
                board_user_attack[i][j] = '_';
            }
            else
            {
                printf("Error in reset_board.\n");
            }
        }
    }
}

void render_board(int board_size, int option)
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
            if (option == BOARD_COMP)
            {
                printf("|_%c_", board_comp[j][i]);
            }
            else if (option == BOARD_USER)
            {
                printf("|_%c_", board_user[j][i]);
            }
            else if (option == BOARD_COMP_ATTACK)
            {
                printf("|_%c_", board_comp_attack[j][i]);
            }
            else if (option == BOARD_USER_ATTACK)
            {
                printf("|_%c_", board_user_attack[j][i]);
            }
            else
            {
                printf("Error in render_board.\n");
            }
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
    if (!CLRSCR)
    {
        return;
    }
    for (int n = 0; n < i; n++)
    {
        printf("\n");
    }
}

/*  Assignment 1: Match
    By Michael Scotson 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **allocate_memory(int height, int width); 
int process_grid(FILE *file, int height, int width, char **line);
int play_game(int height, int width, char **line);
void print_grid(int height, int width, char **line);
int check_input(char input[80], int height, int width, char **line,
        int inRow, int inCol, int inputCount);
int detect_game_over(int height, int width, char **line);
void remove_selection(int inRow, int inCol, char selection, 
        int height, int width, int *count, char **line);
int remove_left(int row, int col, char selection, int height, int width,
        int *count, char **line);
int remove_right(int row, int col, char selection, int height, int width,
        int *count, char **line);
int remove_up(int row, int col, char selection, int height, int width,
        int *count, char **line);
int remove_down(int row, int col, char selection, int height, int width,
        int *count, char **line);
void fill_gaps(int height, int width, char **line);
void shift_left(int height, int width, char **line);
int detect_win(int height, int width, char **line);
void save_grid(char input[80], int height, int width, char **line);
void free_memory(int height, char **line);
[OVERALL] By using a struct to pass around common grid variables, all of these could have been much cleaner and simpler.

int main (int argc, char *argv[]) {
    char *heightArg, *widthArg, *fileArg, *heightNext, *widthNext, **line;
    int playingGame, height, width;
    FILE *file;

    if (argc != 4) {
        fprintf(stderr, "Usage: match height width filename\n");
        return 1;
    }

    heightArg = argv[1];
    widthArg = argv[2];
    fileArg = argv[3];
    height = strtol(heightArg, &heightNext, 10);   
    width = strtol(widthArg, &widthNext, 10);
    
    line = allocate_memory(height, width);
    
    if (height > 999 || width > 999 || height < 2 || width < 2 ||
            *widthNext != '\0' || *heightNext != '\0') {
        fprintf(stderr, "Invalid grid dimensions\n");
        return 2;
    }
    
    file = fopen(fileArg, "r");
    if (file == 0) {
        fprintf(stderr, "Invalid grid file\n");
        return 3;
    }

    if (process_grid(file, height, width, line)) {
        fclose(file);
        return 4;
    }
    fclose(file);
    
    print_grid(height, width, line);
    
    playingGame = 1;
    while (playingGame) { 
        playingGame = play_game(height, width, line);
    }
    
    free_memory(height, line);
    return 0;
}

/*Allocates a 2D array of chars of required height and width and initialises 
each entry to 0*/
char **allocate_memory(int height, int width) {
    int i;
    char **line;
    line = calloc((height + 1), sizeof(char*));
    for (i = 0; i < (height + 1); i++) {
        line[i] = calloc((width + 2), sizeof(char));
    }
    return line;
}
    
/* Checks grid file matches input then loads contents as chars in 2D array
checks for NULL char and rejects if present*/
[COMMENTS] Function comments should explain return values.
int process_grid(FILE *file, int height, int width, char **line) {
    int i, j;  
    
    i = 0;
    while (fgets(line[i], (width + 2), file)) {
        i++;
        if (i > height) {
            fprintf(stderr, "Error reading grid contents\n");
            return 4;
        }
    }

    for (i = 0; i < height; i++) {
        if (line[i][width] != '\n') {
            fprintf(stderr, "Error reading grid contents\n");
            return 4;
        }
        for (j = 0; j < width; j++) {
            if (line[i][j] == 0) {
                fprintf(stderr, "Error reading grid contents\n");
                return 4;
            }
        }
        line[i][width] = '\0'; /*Remove the newline character"*/
        
    }
 
    return 0;
}

/* Checks for valid moves, prints grid, gets input from player and rejects 
or processes valid input. If requested also saves grid to file */
[COMMENTS] Function comments should explain return values.
int play_game(int height, int width, char **line) {
    int inputCount, noPrint, inRow, inCol, count, extra;
    char input[80], selection;

    if (detect_game_over(height, width, line)) {
        return 0;
    }
    
    printf("> ");
    fflush(stdout);

    if (fgets(input, 80, stdin) == NULL) {
        fprintf(stderr, "End of user input\n");
        return 0;
    }

    if (input[strlen(input) - 1] != '\n') {
        while (((extra = getchar()) != EOF) && (extra != '\n'));
    }
    
    inputCount = sscanf(input, "%d %d", &inRow, &inCol);
    if (check_input(input, height, width, line, inRow, inCol, inputCount)) {
        return 1;
    }

    selection = line[inRow][inCol];
    if (selection == '.') {
        return 1;
    }
    
    count = 0;
    remove_selection(inRow, inCol, selection, height, width, &count, line);
    noPrint = 0;
    if (count < 2) {
        line[inRow][inCol] = selection;
        noPrint = 1;
    }

    fill_gaps(height, width, line);
    
    if (noPrint == 0) {
        print_grid(height, width, line);
    }
    noPrint = 0;

    if (detect_win(height, width, line) || detect_game_over(height, width, 
            line)) {
        return 0;
    }
    return 1;
}

/* Prints the grid with a border to the terminal screen  */
void print_grid(int height, int width, char **line) {
    /* function for top and bottom */
    int i, j; 
    printf("+");
    for (i = 0; i < width; i++) {
        printf("-");
    }
    printf("+\n");
    
    for (j = 0; j < height; j++) {
        printf("|%s|\n", line[j]);
    }
    
    printf("+");
    for (i = 0; i < width; i++) {
        printf("-");
    }
    printf("+\n");
    
}

/* Checks input to see if it is requesting a save or if it is invalid*/
int check_input(char input[80], int height, int width, char **line,
        int inRow, int inCol, int inputCount) { 
    if (inputCount != 2) {
        if (input[0] == 'w') {
            save_grid(input, height, width, line);            
        }
        return 1; 
    }
    
    if (inRow < 0 || inRow > (height - 1) || inCol < 0 || 
            inCol > (width - 1)) {
        return 1;
    }
    return 0;
}

/* Returns true (1) if there are no valid moves, false (0) otherwise.
Each char in 2D array (that is not '.' i.e. blank) is checked to see
if there are chars around it that match, if so there are still valid moves */
int detect_game_over(int height, int width, char **line) {
    int i, j;
    char check;
    for (i = 1; i < (height - 1); i++) {
        for (j = 1; j < (width - 1); j++) {
            check = line[i][j];
            if (check != '.') {
                if (check == line[i - 1][j] || check == line[i + 1][j] || 
                        check == line[i][j - 1] || check == line[i][j + 1]) {
                    return 0;
                }
            }
        }
    }
    for (j = 1; j < width; j++) {
        if (line[0][j] == line[0][j - 1]) {
            if (line[0][j] != '.') {
                return 0;
            }
        }
        if (line[height - 1][j] == line[height - 1][j - 1]) {
            if (line[height - 1][j] != '.') {
                return 0;
            }
        }
    }
    for (i = 1; i < height; i++) {
        if (line[i][0] == line[i - 1][0]) {
            if (line[i][0] != '.') {
                return 0;
            }          
        }
        if (line[i][width - 1] == line[i - 1][width - 1]) {
            if (line[i][width - 1] != '.') {
                return 0;
            }
        }
    }
    printf("No moves left\n");
    return 1;
}

/* Removes all characters that match the character selected by the user*/
void remove_selection(int inRow, int inCol, char selection,
        int height, int width, int *count, char **line) {
    remove_left(inRow, inCol, selection, height, width, count, line);
    remove_right(inRow, inCol, selection, height, width, count,
            line);
    remove_up(inRow, inCol, selection, height, width, count, line);
    remove_down(inRow, inCol, selection, height, width, count, line);
}

/* Removes matching characters to the left of input row and column. 
If removed (a match) then characters around new char are removed. This is
done by calling the appropriate functions for the four directions with the 
row and col of new char (including this function- so it is recursive)*/
int remove_left(int row, int col, char selection, int height, int width,
        int *count, char **line) {
    if (col == 0) {
        return 0;
    }
    if (line[row][col - 1] == selection) {
        line[row][col - 1] = '.';
        col--;
        ++*count;
        remove_left(row, col, selection, height, width, count, line);
        remove_right(row, col, selection, height, width, count, line);
        remove_up(row, col, selection, height, width, count, line);
        remove_down(row, col, selection, height, width, count, line);
        return 1; /* am I going to use these return ints */
    }
    return 0;
}

/* Removes matching characters to the right of input row and column.
If removed (a match) then characters around new char are removed. This is
done by calling the appropriate functions for the four directions with the 
row and col of new char (including this function- so it is recursive)*/
int remove_right(int row, int col, char selection, int height, int width,
        int *count, char **line) {
    if (col == width) {
        return 0;
    }
    if (line[row][col + 1] == selection) {
        line[row][col + 1] = '.';
        col++;
        ++*count;
        remove_left(row, col, selection, height, width, count, line);
        remove_right(row, col, selection, height, width, count, line);
        remove_up(row, col, selection, height, width, count, line);
        remove_down(row, col, selection, height, width, count, line);
        return 1;
    }
    return 0;
}

/* Removes matching characters above the input row and column.
If removed (a match) then characters around new char are removed. This is
done by calling the appropriate functions for the four directions with the 
row and col of new char (including this function- so it is recursive)*/
int remove_up(int row, int col, char selection, int height, int width,
        int *count, char **line) {
    if (row == 0) {
        return 0;
    }
    if (line[row - 1][col] == selection) {
        line[row - 1][col] = '.';
        row--;
        ++*count;
        remove_left(row, col, selection, height, width, count, line);
        remove_right(row, col, selection, height, width, count, line);
        remove_up(row, col, selection, height, width, count, line);
        remove_down(row, col, selection, height, width, count, line);
        return 1;
    }
    return 0;
}

/* Removes matching characters below the input row and column.
If removed (a match) then characters around new char are removed. This is
done by calling the appropriate functions for the four directions with the 
row and col of new char (including this function- so it is recursive)*/
int remove_down(int row, int col, char selection, int height, int width,
        int *count, char **line) {
    if (row == height) {
        return 0;
    }
    if (line[row + 1][col] == selection) {
        line[row + 1][col] = '.';
        row++;
        ++*count;
        remove_left(row, col, selection, height, width, count, line);
        remove_right(row, col, selection, height, width, count, line);
        remove_up(row, col, selection, height, width, count, line);
        remove_down(row, col, selection, height, width, count, line);
        return 1;
    }
    return 0;
}

/* This function detects non-blank (not '.') characters above blank ('.')
characters and swaps them (therefore moving the non blank chars down). After
the entire grid is processed the function is called again if a char was moved.
This continues until all non-blank chars have been moved down*/
void fill_gaps(int height, int width, char **line) {
    int i, j, k, moved;
    moved = 0;
    i = 0;
    j = 0;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (line[i][j] == '.' && i > 0) {
                if (line[i][j] != line[(i - 1)][j]) {
                    moved = 1;
                }
                line[i][j] = line[(i - 1)][j];
                line[(i - 1)][j] = '.';
            }
        }
    }
    if (moved == 1) {
        fill_gaps(height, width, line); 
    }
    for (k = 0; k < (width + 2); k++) {
        shift_left(height, width, line);
    }
}

/* Detects blank columns before last column and moves non-blank characters in
column to the right into blank column, column to the right is made blank*/
void shift_left(int height, int width, char **line) {
    int i, j, allBlank;
    allBlank = 1;
    for (i = 0; i < (width - 1); i++) {
        for (j = 0; j < height; j++) {
            if (line[j][i] != '.') {
                allBlank = 0;
            }          
        }
        if (allBlank == 1) {
            for (j = 0; j < height; j++) {
                line[j][i] = line[j][(i + 1)];
                line[j][(i + 1)] = '.';
            }
        }
        allBlank = 1;
    }
}

/* Saves the grid, with border, to a given filename/path */
void save_grid(char input[80], int height, int width, char **line) {
    char filename[79];
    int i;
    FILE *saveFile;
    
    sscanf(input, "w%[^\n]", filename);
    
    saveFile = fopen(filename, "w");
    
    if (saveFile == NULL) {
        fprintf(stderr, "Can not open file for write\n");
    } else {
        fprintf(saveFile, "+");
        for (i = 0; i < width; i++) {
            fprintf(saveFile, "-");
        }
        fprintf(saveFile, "+\n");
        for (i = 0; i < height; i++) {
            fprintf(saveFile, "|%s|\n", line[i]);
        }
        fprintf(saveFile, "+");
        for (i = 0; i < width; i++) {
            fprintf(saveFile, "-");
        }
        fprintf(saveFile, "+\n");
        fprintf(stderr, "Save complete\n");
        fclose(saveFile);
    }
}

/* Returns true (1) if the grid is complete, false (0) if not. Every char
 in the 2D array is checked, if all chars are '.' then it is complete  */
int detect_win(int height, int width, char **line) {
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (line[i][j] != '.') {
                return 0;
            }
        }
    }
    printf("Complete\n");
    return 1;
}

/* Frees the memory allocated for 2D char array line. Is not strictly required
as this happens when program returns, so it's not always called*/
void free_memory(int height, char **line) {
    int i;
    for (i = 0; i < height; i++) {
        free(line[i]);
    }
    free(line);
}

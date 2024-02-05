// Sudoku puzzle verifier and solver
/**
 * By: Yamileth Camacho 
 * CSS 430: Operating systems 
 * 
*/
// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 100

// Struct
typedef struct {
  int row;
  int column;
  int psize;
  int **grid;
  bool *valid;
  bool *complete;
} parameters;

//Functions being used
void *validateRows(void *param);
void *validateCol(void *param);
void *validateSquare(void *param);
bool completePuzzle(int **grid, int psize);
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid);


// Validates each row in the puzzle
void *validateRows(void *param) {
  parameters *params = (parameters *)param;
  int row = params->row;
  int psize = params->psize;
  int **grid = params->grid;

  int validityArray[psize + 1];
  for (int i = 1; i <= psize; i++) {
    validityArray[i] = 0;
  }

  for (int col = 1; col <= psize; col++) {
    int num = grid[row][col];
    if (num < 1 || num > psize || validityArray[num] == 1) {
      *(params->valid) = false;
      pthread_exit(NULL);
    } else {
      validityArray[num] = 1;
    }
  }

  pthread_exit(NULL);
}

// Validates each column in the puzzle
void *validateCol(void *param) {
  parameters *params = (parameters *)param;
  int col = params->column;
  int psize = params->psize;
  int **grid = params->grid;

  int validityArray[psize + 1];
  for (int i = 1; i <= psize; i++) {
    validityArray[i] = 0;
  }

  for (int row = 1; row <= psize; row++) {
    int num = grid[row][col];
    if (num < 1 || num > psize || validityArray[num] == 1) {
      *(params->valid) = false;
      pthread_exit(NULL);
    } else {
      validityArray[num] = 1;
    }
  }

  pthread_exit(NULL);
}

// Validates each square/ subgrid in the puzzle
void *validateSquare(void *param) {

  parameters *params = (parameters *)param;
  int row = params->row;
  int col = params->column;

  // checks if each subgrid is valid
  if (row > params->psize || row % (params->psize / 3) != 1 ||
      col > params->psize || col % (params->psize / 3) != 1) {
    pthread_exit(NULL);
  }

  int psize = params->psize;
  int **grid = params->grid;

  int *validityArray = (int *)calloc(psize + 1, sizeof(int));
  int i, j;

  for (i = row; i < row + psize / 3; i++) {
    for (j = col; j < col + psize / 3; j++) {
      int num = grid[i][j];
      if (num < 1 || num > psize || validityArray[num] == 1) {
        free(validityArray);
        pthread_exit(NULL);
      } else {
        validityArray[num] = 1;
      }
    }
  }

  free(validityArray);
  pthread_exit(NULL);
}

//Fills the easy puzzles that have 0's
bool completePuzzle(int **grid, int psize) {
  for (int i = 1; i <= psize; i++) {
    for (int j = 1; j <= psize; j++) {
      if (grid[i][j] == 0) {
      
        for (int num = 1; num <= psize; num++) {
          // Check if number is not present in the current row, column, and subgrid
          bool isValid = true;
          for (int k = 1; k <= psize; k++) {
            if (grid[i][k] == num || grid[k][j] == num ||
                grid[(i - 1) / 3 * 3 + k / 3 + 1][(j - 1) / 3 * 3 + k % 3 + 1] == num) {
              isValid = false;
              break;
            }
          }
          // If its valid place the number 
          if (isValid) {
            grid[i][j] = num;
            //fill the remaining puzzle
            if (completePuzzle(grid, psize)) {
              return true;
            }
            grid[i][j] = 0;
          }
        }
        return false;  // No valid number found 
      }
    }
  }
  return true;  
}

// Creates threads to check each row, column, and subgrid in the puzzle
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  // Initialize thread variables
  pthread_t threads[MAX_THREADS];
  parameters params[MAX_THREADS + 1];

  // Initialize flags
  *complete = true;
  *valid = true;

  // Checks if it is complete
  // IF not complete no need to check validity
  for (int i = 1; i <= psize; i++) {
    for (int j = 1; j <= psize; j++) {
      if (grid[i][j] == 0) {
        *complete = false;
        return;
      }
    }
  }

  // Create threads for row validation
  for (int i = 1; i <= psize; i++) {
    params[i].row = i;
    params[i].column = 0;
    params[i].psize = psize;
    params[i].grid = grid;
    params[i].valid = valid;
    params[i].complete = complete;
    pthread_create(&threads[i], NULL, validateRows, &params[i]);
  }

  // Create threads for column validation
  for (int i = 1 + psize; i <= 2 * psize; i++) {
    params[i].row = 0;
    params[i].column = i - psize;
    params[i].psize = psize;
    params[i].grid = grid;
    params[i].valid = valid;
    params[i].complete = complete;
    pthread_create(&threads[i], NULL, validateCol, &params[i]);
  }

  // Create threads for square validation
  for (int i = 1 + 2 * psize; i <= 3 * psize; i++) {
    int subgridIndex = i - 2 * psize - 1;
    params[i].row = ((subgridIndex - 1) / (psize / 3)) * (psize / 3) + 1;
    params[i].column = ((subgridIndex - 1) % (psize / 3)) * (psize / 3) + 1;
    params[i].psize = psize;
    params[i].grid = grid;
    params[i].valid = valid;
    params[i].complete = complete;
    pthread_create(&threads[i], NULL, validateSquare, &params[i]);
  }

  // Join threads
  for (int i = 1; i <= 3 * psize; i++) {
    pthread_join(threads[i], NULL);
  }
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  // fills puzzle 
  completePuzzle(grid, sudokuSize);

  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}

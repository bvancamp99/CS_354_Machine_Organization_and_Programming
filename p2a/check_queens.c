////////////////////////////////////////////////////////////////////////////////
// Main File:        check_queens.c
// This File:        check_queens.c
// Other Files:      N/A
// Semester:         CS 354 SPRING 2019
//           
// Author:           Bryce Van Camp
// Email:            bvancamp@wisc.edu
// CS Login:         bvan-camp
//           
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//           
// Persons:          N/A
//           
// Online sources:   N/A
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h> 
#include <stdlib.h>  
#include <string.h>   

char *COMMA = ",";

/* COMPLETED:
 * Retrieves from the first line of the input file,
 * the number of rows and columns for the board.
 *
 * fp: file pointer for input file
 * rows: pointer to number of rows
 * cols: pointer to number of columns
 */
void get_dimensions(FILE *fp, int *rows, int *cols)
{
	char *line = NULL;
	size_t len = 0;
	if (getline(&line, &len, fp) == -1)
	{
		printf("Error in reading the file\n");
		exit(1);
	}

	char *token = NULL;
	token = strtok(line, COMMA);
	*rows = atoi(token);

	token = strtok(NULL, COMMA);
	*cols = atoi(token);
}

/* Helper function for check_queens that returns 1 if there
 * is another queen on the same row, column, or diagonal
 * path that the current queen is on, and returns 0 otherwise.
 *
 * board: heap allocated 2D board
 * numRows: number of rows
 * numCols: number of columns
 * curRow: current row of the queen
 * curCol: current column of the queen
 */
int check_queensHelper(int **board, int numRows, int numCols, int curRow,
	int curCol)
{
	//Another queen on curCol?

	for (int i = 0; i < curRow; i++)
	{
		//check if same column has a queen from row 0 to curRow-1
		if (*(*(board + i) + curCol) == 1)
		{
			return 1;
		}
	}

	for (int i = numRows - 1; i > curRow; i--)
	{
		//check if same column has a queen from curRow+1 to numRows-1
		if (*(*(board + i) + curCol) == 1)
		{
			return 1;
		}
	}

	//Another queen on curRow?

	for (int j = 0; j < curCol; j++)
	{
		//check if same row has a queen from col 0 to curCol-1
		if (*(*(board + curRow) + j) == 1)
		{
			return 1;
		}
	}

	for (int j = numCols - 1; j > curCol; j--)
	{
		//check if same row has a queen from curCol+1 to numCols-1
		if (*(*(board + curRow) + j) == 1)
		{
			return 1;
		}
	}

	//Queen on negative slope diagonal path?

	int i = curRow + 1;
	int j = curCol + 1;
	while (i < numRows && j < numCols)
	{
		if (*(*(board + i) + j) == 1)
		{
			return 1;
		}

		i++;
		j++;
	}

	i = curRow - 1;
	j = curCol - 1;
	while (i >= 0 && j >= 0)
	{
		if (*(*(board + i) + j) == 1)
		{
			return 1;
		}

		i--;
		j--;
	}

	// Queen on positive slope diagonal path?

	i = curRow - 1;
	j = curCol + 1;
	while (i >= 0 && j < numCols)
	{
		if (*(*(board + i) + j) == 1)
		{
			return 1;
		}

		i--;
		j++;
	}

	i = curRow + 1;
	j = curCol - 1;
	while (i < numRows && j >= 0)
	{
		if (*(*(board + i) + j) == 1)
		{
			return 1;
		}

		i++;
		j--;
	}

	//return 0 if no queens were found on any of the paths
	return 0;
}

/* COMPLETED:
 * Returns 1 if and only if there exists at least one pair
 * of queens that can attack each other.
 * Otherwise returns 0.
 *
 * board: heap allocated 2D board
 * rows: number of rows
 * cols: number of columns
 */
int check_queens(int **board, int rows, int cols)
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (*(*(board + i) + j) == 1)
			{
				if (check_queensHelper(board, rows, cols, i, j) == 1)
				{
					return 1;
				}
			}
		}
	}

	//return 0 if no queens can attack each other
	return 0;
}


/* COMPLETED:
 * This program prints true if the input file has any pair
 * of queens that can attack each other, and false otherwise
 *
 * argc: CLA count
 * argv: CLA value
 */
int main(int argc, char *argv[])
{
	//Check if number of command-line arguments is correct.
	if (argc != 2)
	{
		printf("Usage: ./check_queens <input_filename>\n");
		exit(1);
	}


	//Open the file and check if it opened successfully.
	FILE *fp = fopen(*(argv + 1), "r");
	if (fp == NULL)
	{
		printf("Cannot open file for reading\n");
		exit(1);
	}


	//Declare local variables.
	int rows, cols;

	//Call get_dimensions to retrieve the board dimensions.
	get_dimensions(fp, &rows, &cols);

	//Dynamically allocate a 2D array of dimensions retrieved above.
	int **board = malloc(sizeof(int*) * rows);
	if (board == NULL)
	{
		printf("Error allocating memory to the board\n");
		exit(1);
	}
	for (int i = 0; i < rows; i++)
	{
		*(board + i) = malloc(sizeof(int) * cols);
		if (*(board + i) == NULL)
		{
			printf("Error allocating memory to row %i of the board\n", i);
			exit(1);
		}
	}

	//Read the file line by line.
	//Tokenize each line wrt comma to store the values in your 2D array.
	char *line = NULL;
	size_t len = 0;
	char *token = NULL;
	for (int i = 0; i < rows; i++)
	{
		if (getline(&line, &len, fp) == -1)
		{
			printf("Error while reading the file\n");
			exit(1);
		}

		token = strtok(line, COMMA);
		for (int j = 0; j < cols; j++)
		{
			//Initialize 2D array.
			*(*(board + i) + j) = atoi(token);
			token = strtok(NULL, COMMA);
		}
	}

	//Call the function check_queens and print the appropriate
	//output depending on the function's return value.
	int result = check_queens(board, rows, cols);
	if (result == 1)
	{
		printf("true\n");
	}
	else if (result == 0)
	{
		printf("false\n");
	}

	//Free all dynamically allocated memory.
	for (int i = 0; i < rows; i++)
	{
		free(*(board + i));
	}
	free(board);
	board = NULL;
	free(line);
	line = NULL;

	//Close the file.
	if (fclose(fp) != 0)
	{
		printf("Error while closing the file\n");
		exit(1);
	}

	return 0;
}

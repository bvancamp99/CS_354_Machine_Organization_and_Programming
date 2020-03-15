
////////////////////////////////////////////////////////////////////////////////
// Main File:        traverse_spiral.c - traverses a spiral of a square of nums
// This File:        traverse_spiral.c
// Other Files:      N/A
// Semester:         CS 354 Spring 2019
//
// Author:           Bryce Van Camp
// Email:            bvancamp@wisc.edu
// CS Login:         bvan-camp
//         
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          none
//
// Online sources:   none
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *COMMA = ","; 

// Structure representing Matrix
// size: Dimension of the square (size*size)
// array: 2D array of integers
typedef struct _Square
{ 
    int size;
    int **array;
} Square; 

/* COMPLETED:
 * Retrieves from the first line of the input file,
 * the size of the square matrix.
 * 
 * fp: file pointer for input file
 * size: pointer to size of the square matrix
 */
void get_dimensions(FILE *fp, int *size)
{   
	char *line = NULL;
	size_t len = 0;
	if (getline(&line, &len, fp) == -1)
	{
		printf("error retrieving size from file\n");
		exit(1);
	}
	*size = atoi(line);
}

/* COMPLETED:
 * Traverses a given layer from the square matrix
 * 
 * array: Heap allocated 2D square matrix
 * size: size of the 2D square matrix
 * layer: layer number to be traversed
 * op: pointer to the output file
 */
void traverse_layer(int **array, int size, int layer, FILE *op)
{  
	int row_num, col_num;
	int i,j;
	
	//corner case: size is odd & the layer is last so only one entry to print
        if(size % 2 == 1 && layer == (size + 1) / 2 - 1){
                fprintf(op, "%d\n", *(*(array + layer) + layer));
                return;
        }

	//Traverse upper row from left to right with appropriate bounds
	row_num = layer;
	for (j = layer; j < size-layer-1; j++)
	{
		fprintf(op, "%i ", *(*(array + row_num) + j));
	}

	//Traverse right column from top to bottom with appropriate bounds
	col_num = size-layer-1;
	for (i = layer; i < size-layer-1; i++)
	{
		fprintf(op, "%i ", *(*(array + i) + col_num));
	}

	//Traverse lower row from right to left with appropriate bounds
	row_num = size-layer-1;
	for (j = size-layer-1; j >= layer + 1; j--)
	{
		fprintf(op, "%i ", *(*(array + row_num) + j));
	}

	//Traverse left column from bottom to top with appropriate bounds
	col_num = layer;
	for (i = size-layer-1; i >= layer + 1; i--)
	{
		fprintf(op, "%i ", *(*(array + i) + col_num));
	}
}


/* COMPLETED:
 * Traverses the square matrix spirally
 * 
 * square: pointer to a structure that contains 2D square matrix
 * op: pointer to the output file
 */
void traverse_spirally(Square *square, FILE *op)
{         
	int size = square->size; 
	int num_layers = 0;   
	num_layers = size/2; 
	if(size%2 == 1) {
		num_layers++;
	}
	 
	int i;
	for(i = 0; i < num_layers; i++) {
		traverse_layer(square->array, size, i, op);
	}
}

/* COMPLETED:
 * This program reads a square matrix from the input file
 * and outputs its spiral order traversal to the output file
 *
 * argc: CLA count
 * argv: CLA value
 */
  
int main(int argc, char *argv[])
{        
	//Check if number of command-line args is correct
	if (argc != 3)
	{
		printf("Usage: ./traverse_spiral <input_filename> <output_filename>\n");
		exit(1);
	}

	//Open the file and check if it opened successfully
	FILE *inputFile = fopen(*(argv + 1), "r");
	if (inputFile == NULL)
	{
		printf("cannot open input file for reading\n");
		exit(1);
	}

	//Declare struct of type Square that contains size and 2D array
	Square square;

	//Call the function get_dimensions to retrieve size of the square matrix
	get_dimensions(inputFile, &(square.size));

	//Dynamically allocate a 2D array as per the retrieved dimensions
	square.array = malloc(sizeof(int*) * square.size);
	if (square.array == NULL)
	{
		printf("error allocating memory to the array\n");
		exit(1);
	}
	for (int i = 0; i < square.size; i++)
	{
		*(square.array + i) = malloc(sizeof(int) * square.size);
		if (*(square.array + i) == NULL)
		{
			printf("error allocating memory to row %i of the array\n", i);
			exit(1);
		}
	}

	//Read the file line by line by using the function getline as used in get_dimensions
	//Tokenize each line wrt comma to store the values in the square matrix
	char *line = NULL;
	size_t len = 0;
	char *token = NULL;
	for (int i = 0; i < square.size; i++)
	{
		if (getline(&line, &len, inputFile) == -1)
		{
			printf("error retrieving line %i of array in file\n", i);
			exit(1);
		}

		token = strtok(line, COMMA);
		for (int j = 0; j < square.size; j++)
		{
			*(*(square.array + i) + j) = atoi(token);
			token = strtok(NULL, COMMA);
		}
	}

	//Open the output file
	FILE *outputFile = fopen(*(argv + 2), "w");
	if (outputFile == NULL)
	{
		printf("error opening output file\n");
		exit(1);
	}

	//Call the function traverse_spirally
	traverse_spirally(&square, outputFile);

	//Free the dynamically allocated memory
	free(line);
	line = NULL;
	for (int i = 0; i < square.size; i++)
	{
		free(*(square.array + i));
	}
	free(square.array);
	square.array = NULL;

	//Close the input file
	if (fclose(inputFile) != 0)
	{
		printf("error closing input file\n");
		exit(1);
	}

	//Close the output file
	if (fclose(outputFile) != 0)
	{
		printf("error closing output file\n");
		exit(1);
	}

	return 0;
}

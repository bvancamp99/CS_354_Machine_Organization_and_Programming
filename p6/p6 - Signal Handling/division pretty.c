////////////////////////////////////////////////////////////////////////////////
// Main File:        intdate.c - handles misc signals and prints pid/time
// This File:        division.c
// Other Files:      intdate.c, sendsig.c
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
//////////////////////////// 80 columns wide ///////////////////////////////////

#include <signal.h> //sigaction()
#include <stdio.h> //printf()
#include <stdlib.h> //exit()
#include <string.h> //memset()

int divcount; //global counter for number of divisions successfully completed

/*
 * This function handles the floating-point exception signal SIGFPE.  It prints
 * some messages and exits successfully.  There are no params, and return type
 * is void.
 */
void handler_SIGFPE()
{
	//print messages upon receiving SIGFPE signal
	printf("Error: a division by 0 operation was attempted.\n");
	printf("Total number of operations completed successfully: %i\n", 
		divcount);
	printf("The program will be terminated.\n");

	//exit after printing messages
	exit(0);
}

/*
 * This function handles the interrupt signal SIGINT.  It prints some messages
 * and exits successfully.  There are no params, and return type is void.
 */
void handler_SIGINT()
{
	//print messages upon receiving SIGINT signal
	printf("\nTotal number of operations successfully completed: %i\n", 
		divcount);
	printf("The program will be terminated.\n");

	//exit after printing messages
	exit(0);
}

/*
 * This is the main function that registers the signal handlers, along with
 * providing functionality for division.  There are no params.
 *
 * Returns: val of %eax, which is 0 upon successful completion.  We won't
 * reach the return statement in this program, however.
 */
int main()
{
	//declare variables for use in division input and output
	int dividend, divisor, quotient, remainder;
	char input[100];

	//register a signal handler for SIGFPE
	struct sigaction act_divzero;
	memset(&act_divzero, 0, sizeof(act_divzero));
	act_divzero.sa_handler = handler_SIGFPE;

	//perform sigaction on SIGFPE and check ret val
	if (sigaction(SIGFPE, &act_divzero, NULL) == -1)
	{
		printf("error on sigaction() for SIGFPE\n");
		exit(1);
	}

	//register a signal handler for SIGINT
	struct sigaction act_int;
	memset(&act_int, 0, sizeof(act_int));
	act_int.sa_handler = handler_SIGINT;

	//perform sigaction on SIGINT and check ret val
	if (sigaction(SIGINT, &act_int, NULL) == -1)
	{
		printf("error on sigaction() for SIGINT\n");
		exit(1);
	}

	while (1)
	{
		//prompt for dividend user input
		printf("Enter first integer: ");
		if (!fgets(input, 100, stdin))
		{
			printf("error on fgets() for first integer\n");
			exit(1);
		}
		dividend = atoi(input);

		//prompt for divisor input
		printf("Enter second integer: ");
		if (!fgets(input, 100, stdin))
		{
			printf("error on fgets() for second integer\n");
			exit(1);
		}
		divisor = atoi(input);

		//find quotient and remainder
		quotient = dividend / divisor;
		remainder = dividend % divisor;

		//print results of operation
		printf("%i / %i is %i with a remainder of %i\n", dividend, divisor, 
			quotient, remainder);

		//increment global counter
		divcount++;
	}

	//shouldn't ever reach this point
	return 0;
}
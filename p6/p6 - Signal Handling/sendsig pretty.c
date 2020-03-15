////////////////////////////////////////////////////////////////////////////////
// Main File:        intdate.c - handles misc signals and prints pid/time
// This File:        sendsig.c
// Other Files:      intdate.c, division.c
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

#include <signal.h> //kill()
#include <stdio.h> //printf()
#include <stdlib.h> //exit()
#include <string.h> //memset()

/*
 * This is the main function that registers the signal handlers, along with
 * providing functionality for division.
 *
 * int argc - number of command-line args
 * char *argv[] - stores command-line args
 *
 * Returns: val of %eax, which is 0 upon successful completion.  We won't
 * reach the return statement in this program, however.
 */
int main(int argc, char* argv[])
{
	//check for valid command-line args
	if (argc != 3 || (strcmp(argv[1], "-i") != 0 && strcmp(argv[1], "-u") 
		!= 0))
	{
		printf("Usage: <signal type> <pid>\n");
		exit(1);
	}

	//store pid
	pid_t pid = atoi(argv[2]);

	//send SIGINT signal if command-line arg is "-i"
	if (strcmp(argv[1], "-i") == 0)
	{
		if (kill(pid, SIGINT) == -1)
		{
			printf("error on kill() for SIGINT\n");
			exit(1);
		}
	}
	//else send SIGUSR1 signal on "-u" command-line arg
	else
	{
		if (kill(pid, SIGUSR1) == -1)
		{
			printf("error on kill() for SIGUSR1\n");
			exit(1);
		}
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////
// Main File:        intdate.c - handles misc signals and prints pid/time
// This File:        intdate.c
// Other Files:      sendsig.c, division.c
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
#include <time.h> //time() and ctime()
#include <unistd.h> //alarm()

pid_t pid; //declare process id
time_t rawtime; //declare time var
int user1count; //declare global counter for number of SIGUSR1 signals caught

/*
 * This function handles the alarm signal SIGALRM.  It prints the process id
 * and current time, and re-arms the alarm.  There are no params, and return
 * type is void.
 */
void handler_SIGALRM()
{
	//store current time
	time(&rawtime);

	//print pid and time in the same format as the Linux date command
	printf("PID: %i | Current Time: %s", pid, ctime(&rawtime));

	//re-arm the alarm to go off again three seconds later
	alarm(3);
}

/*
 * This function handles the user signal SIGUSR1.  It increments our global
 * counter user1count and prints a message.  There are no params, and return
*  type is void.
 */
void handler_SIGUSR1()
{
	//increment counter
	user1count++;

	//print message
	printf("SIGUSR1 caught!\n");
}

/*
 * This function handles the interrupt signal SIGINT.  It prints some messages
 * and exits successfully.  There are no params, and return type is void.
 */
void handler_SIGINT()
{
	//print messages upon receiving SIGINT signal
	printf("SIGINT received.\n");
	printf("SIGUSR1 was received %i times. Exiting now.\n", user1count);

	//exit after printing messages
	exit(0);
}

/*
 * This is the main function that registers the signal handlers, along with a
 * loop printing the process id and current time.  There are no params.
 *
 * Returns: val of %eax, which is 0 upon successful completion.  We won't
 * reach the return statement in this program, however.
 */
int main()
{
	//register a signal handler for SIGALRM
	struct sigaction act_alarm;
	memset(&act_alarm, 0, sizeof(act_alarm));
	act_alarm.sa_handler = handler_SIGALRM;

	//perform sigaction on SIGALRM and check ret val
	if (sigaction(SIGALRM, &act_alarm, NULL) == -1)
	{
		printf("error on sigaction() for SIGALRM\n");
		exit(1);
	}

	//register a signal handler for SIGUSR1
	struct sigaction act_user1;
	memset(&act_user1, 0, sizeof(act_user1));
	act_user1.sa_handler = handler_SIGUSR1;

	//perform sigaction on SIGUSR1 and check ret val
	if (sigaction(SIGUSR1, &act_user1, NULL) == -1)
	{
		printf("error on sigaction() for SIGUSR1\n");
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

	//set an alarm that will go off after 3 seconds
	alarm(3);

	//store pid once
	pid = getpid();

	//print text
	printf("Pid and time will be printed every 3 seconds.\n");
	printf("Enter ^C to end the program.\n");

	//enter infinite loop
	while (1)
		;

	//shouldn't ever reach this point
	return 0;
}
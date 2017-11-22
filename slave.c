#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include <error.h>
#include <assert.h>

//void CTRLhandler(int sig); // for the ctrl^C
//void TimeHandler(int sig); // for alarm (z)
//
//char *shm, *sharedNum;
//int slaveNum = 0, shmid;
//
//int main(int argc, char **argv)
//{
//	// I'm a child (Slave) now what do I do with the data that's being passed?
//	// Let's assign all the data to variables I can use!
//	// What's the first thing that I passed from master ?
//	// I passed filename as a char, well filenames are ment to be char so lets keep it that way
//	signal(SIGTERM, TimeHandler); // getting is alarm is activated
//	signal(SIGQUIT,CTRLhandler); //getting if ctrl^c called
//	signal(SIGINT, CTRLhandler); // for funs
//
//		const char *filename = argv[0]; // This is the same as me in the comment line doing ./Slave filename.txt (exec super cool)
//		// What's else was passed?
//		// Oh wait i passed a char which needs to be an int so I can use it
//		// Let's Define it as an int and convert the char to an int
//		int max_writes = atoi(argv[1]); //now we have max writes
//		int slaveNum = atoi(argv[2]);
//		int numOfSlaves = atoi(argv[3]);
////		printf("This slave num is %d", slaveNum);
//		// Now let's have the slave print something to see if it worked
//		// Let's print its pid and let's see if everything passed correctly
//		// HEY THE OUTPUT WILL LOOK WONKY that's ok that's an issue you won't have to worry about
////		pid_t pid;
////		pid = getpid();
//
////	printf("\nI am a slave I was passed filename : %s  and max_writes : %d my pid is : %d\n", filename,max_writes,pid);
//
//	/*  */
//	int i;
//	key_t key = atoi(argv[4]);
//	shmid = atoi(argv[6]);
//	/*
//     * Locate the segment.
//     */
//		const int SHMSZ = sizeof(int);
////		if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
////		perror("shmget");
////		exit(1);
////	}
////
////	/*
////     * Now we attach the segment to our data space.
////     */
////	if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1) {
////		perror("shmat");
////		exit(1);
////	}
//
//
////	printf("this is the intNum: %s\n", sharedNum);
//
//
//	/*  */
//

FILE* fileWrite(char *fileName);
int random_number(int min_num, int max_num);
void CTRLhandler(int sig); // for the ctrl^C
void TimeHandler(int sig); // for alarm (z)

int slaveNum = 0; // the x number of slaves is global so the CTRLhandler and TimeHandler can access it
int shmid;
#define SHMSZ sizeof(int)


int main(int argc, char *argv[])
{
	signal(SIGTERM, TimeHandler); // getting is alarm is activated
	signal(SIGQUIT,CTRLhandler); //getting if ctrl^c called
	signal(SIGINT, CTRLhandler);

	char *filename = argv[0];  // filename to write too
	int max_writes = atoi(argv[1]); //the Y value in the master
	slaveNum = atoi(argv[2]); // the x value in master (amount of slaves)
	key_t key = atoi(argv[3]); // making key for access
	shmid = atoi(argv[4]);
	int n = atoi(argv[5]);

	typedef struct PeterStruct // shared memory
	{
		int sum;
		int turn;
		enum state { idle, want_in, in_cs } flag[];
	}peter;

	if((shmid = shmget(key, SHMSZ, IPC_CREAT | 0600)) < 0)  // creating the shared memory
	{
		perror("shmget");
		exit(1);
	}
	peter *shareData;
	shareData = (peter*)shmat(shmid, NULL,0);
	if (shareData->sum ==(int)-1) // Now we attach the segment to our data space.
	{
		perror("Shmat error in Main");
		exit(1);
	}

	char buff[100];  // for the time

	int j;
	int counter = 0;
	for (counter = 0; counter < max_writes; counter++)
	{
		do
		{
			shareData->flag[slaveNum] = want_in; // Raise my flag
			j = shareData->turn; // Set local variable
			// wait until its my turn
			while ( j != slaveNum)
			{
				j = (shareData->flag[j] != idle ) ? shareData->turn : ( j + 1 ) % n;
			}
			// Declare intention to enter critical section
			shareData->flag[slaveNum] = in_cs;
			// Check that no one else is in critical section
			for ( j = 0; j < n; j++ )
			{
				if ( ( j != slaveNum) && (shareData->flag[j] == in_cs ) )
					break;
			}
		} while ( ( j < n ) || ( shareData->turn != slaveNum && shareData->flag[shareData->turn] != idle ) );
		// Assign turn to self and enter critical section
		shareData->turn = slaveNum;
		//Critical secion
		printf("Process %d Entering Critical Section.\n",slaveNum+1);
		sleep(rand()%2); // this was only don't 0 or 1 so 0,3 leads to 0-2 seconds for sleep
		FILE *fp = fopen(filename, "a"); // opening file to write into
		shareData->sum+=1;
		time_t now = time(0); // getting time
		strftime(buff, 100, "%H:%M:%S",localtime(&now)); //I included Seconds because I like it that way
		fprintf(fp, "File modified by process number %d at time %d with sharedNum = %c\n", slaveNum+1, buff, shareData->sum);
		printf("File modified by process number %d at time %d with sharedNum = %c\n", slaveNum+1, buff, shareData->sum);
		fclose(fp);
		sleep(rand()%2);
		printf("Process %d Exiting Critical Section.\n", slaveNum+1);
		//Exit critical section
		j = (shareData->turn + 1) % n;
		while (shareData->flag[j] == idle)
			j = (j + 1) % n;
		// Assign turn to next waiting process; change own flag to idle
		shareData->turn = j; shareData->flag[slaveNum] = idle;
	}
	return 0;
}

void CTRLhandler(int sig)
{
	signal(sig, SIG_IGN); // ignoring any signal passed to the CTRLhandler
	fprintf(stderr, "\nCtrl^C Called, Process %d Exiting\n", slaveNum+1);
	if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
	{
		perror("Error In shmdt Child CTRLhandler");
	}
	kill(getpid(), SIGKILL);
}
void TimeHandler(int sig)
{
	if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
	{
		perror("Error In shmdt Child TimeHandler");
	}
	//shmctl(shmid, IPC_RMID, NULL); //mark shared memory for deletion
	fprintf(stderr, "\nOut of Time, Process %d Exiting\n", slaveNum+1);
	kill(getpid(), SIGKILL);
	//exit(0);
}


//
//
//		FILE *fp;
//		fp = fopen(filename, "a");
////		shm = sharedNum;
////    int level[numOfSlaves];
////    int last_to_enter[numOfSlaves - 1];
//    int j;
////	tempNum = shm[1] - '0';
//
//
//	typedef struct PeterStruct // shared memory
//	{
//		int sum;
//		int turn;
//		enum state { idle, want_in, in_cs } flag[numOfSlaves];
//	}peter;
//
//	peter *shareData;
//
//	if((shmid = shmget(key, SHMSZ, IPC_CREAT | 0600)) < 0)  // creating the shared memory
//	{
//		perror("shmget");
//		exit(1);
//	}
//	shareData = (peter*)shmat(shmid, NULL,0);
//	if (shareData->sum ==(int)-1) // Now we attach the segment to our data space.
//	{
//		perror("Shmat error in Main");
//		exit(1);
//	}
//
//
//
//
//
//
//
//	time_t timeModified;
//	int counter = 0;
//	for (counter = 0; counter < max_writes; counter++)
//	{
//		do
//		{
//			shareData->flag[slaveNum] = want_in; // Raise my flag
//			j = shareData->turn; // Set local variable
//			// wait until its my turn
//			while ( j != slaveNum)
//			{
//				j = (shareData->flag[j] != idle ) ? shareData->turn : ( j + 1 ) % numOfSlaves;
//			}
//			// Declare intention to enter critical section
//			shareData->flag[slaveNum] = in_cs;
//			// Check that no one else is in critical section
//			for ( j = 0; j < numOfSlaves; j++ )
//			{
//				if ( ( j != slaveNum) && (shareData->flag[j] == in_cs ) )
//					break;
//			}
//		} while ( ( j < numOfSlaves ) || ( shareData->turn != slaveNum && shareData->flag[shareData->turn] != idle ) );
//		// Assign turn to self and enter critical section
//		shareData->turn = slaveNum;
//		//Critical secion
//		printf("Process %d Entering Critical Section.\n",slaveNum);
//		sleep(1); // this was only don't 0 or 1 so 0,3 leads to 0-2 seconds for sleep
////		FILE *fp = fileWrite(filename); // opening file to write into
//		FILE *fp;
//		fp = fopen(filename, "a");
//		shareData->sum+=1;
//		timeModified = time(NULL);
//		fprintf(fp, "File modified by process number %d at time %d with sharedNum = %c\n", slaveNum, timeModified, shm[0]);
//             printf("File modified by process number %d at time %d with sharedNum = %c\n", slaveNum, timeModified, shm[0]);
//		fclose(fp);
//		sleep(1);
//		printf("Process %d Exiting Critical Section.\n", slaveNum+1);
//		//Exit critical section
//		j = (shareData->turn + 1) % numOfSlaves;
//		while (shareData->flag[j] == idle)
//			j = (j + 1) % numOfSlaves;
//		// Assign turn to next waiting process; change own flag to idle
//		shareData->turn = j; shareData->flag[slaveNum] = idle;
//	}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
////	for (i = 0; i < max_writes; i++ )
////		 {
////			 printf("value of position before while loop: %d, process: %d", tempNum, slaveNum);
////            while (tempNum != slaveNum) {
////				tempNum = shm[1] - '0';
////                    printf("Slave %d waiting...\n", slaveNum);
////                    sleep(1);
////            }
////             sleep(1);
////             tempNum = shm[0] - '0';
////             tempNum++;
////             shm[0] = tempNum + '0';
////             fprintf(fp, "File modified by process number %d at time %d with sharedNum = %c\n", slaveNum, timeModified, shm[0]);
////             printf("File modified by process number %d at time %d with sharedNum = %c\n", slaveNum, timeModified, shm[0]);
////             sleep(1);
////            tempNum = sharedNum[1] - '0';
////            tempNum++;
////             if(tempNum > numOfSlaves)
////                 tempNum = 1;
////             shm[1] = tempNum + '0';
////            sprintf(sharedNum[1], "%d", tempNum);
//
//             // execute code to enter critical section;
//
////			 int charge;
////			 while(charge != slaveNum)
////			 {
////				 int j;
////				 charge = 0;
////				 for (j = 1; j <= numOfSlaves; j++) {
////					 if (shm[j] == '0')
////						 charge++;
////				 }
////				 // busy wait
////				 if(charge != (numOfSlaves)) {
////					 printf("Slave %d waiting...\n", slaveNum);
////					 sleep(1);
////				 }
////			 }
//
//			 /* Critical section */
//			// sleep for random amount of time (between 0 and 2 seconds);
//
//			// increment sharedNum
////			 int intNum = atoi(sharedNum);
////			 intNum++;
////			 sprintf(sharedNum, "%d", intNum);
////             *sharedNum += 1;
//
//			// write message into the file
//
//			// sleep for random amount of time (between 0 and 2 seconds);
////			sleep(1);
//			// exit from critical section;
////			 shm[slaveNum] = '0';
////		}
//		fclose(fp);
//
//		i = shmdt(shm);
//		if(i == -1)
//		{
//			perror("shmop: shmdt failed");
//		} else
//		{
//			(void) fprintf(stderr, "shmop: shmdt returned %d\n", i);
//		}
//    return(0);
//}
//void CTRLhandler(int sig)
//{
//	signal(sig, SIG_IGN); // ignoring any signal passed to the CTRLhandler
//	fprintf(stderr, "\nCtrl^C Called, Process %d Exiting\n", slaveNum+1);
//	if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
//	{
//		perror("Error In shmdt Child CTRLhandler");
//	}
//	kill(getpid(), SIGKILL);
//}
//void TimeHandler(int sig)
//{
//	if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
//	{
//		perror("Error In shmdt Child TimeHandler");
//	}
//	//shmctl(shmid, IPC_RMID, NULL); //mark shared memory for deletion
//	fprintf(stderr, "\nOut of Time, Process %d Exiting\n", slaveNum);
//	kill(getpid(), SIGKILL);
//	//exit(0);
//}
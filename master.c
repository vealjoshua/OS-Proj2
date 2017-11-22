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

//void slaveProcess(pid_t pid);
//void masterProcess(int slaves);
//void CTRLhandler(int signum);  // handeling Ctrl^C signal
//void on_alarm(int signal);  // handeling the alarm(z) signal
//
//pid_t pid[20];
//int c, numOfSlaves = 5, max_writes = 3, terminateTime = 20, status, shmid;
//char *filename = "test.out";
//
//key_t key = 1995;
//char *shm, *sharedNum;
//
//int main(int argc, char **argv) {
//    signal (SIGINT,CTRLhandler);
//    signal(SIGALRM,on_alarm);
//    alarm(terminateTime);
//
//    while ((c = getopt(argc, argv, "hi:l:s:t:")) != -1)
//        switch (c) {
//            case 'h':
//                printf("Command Line Argument Functions\n"
//                               "-h: Display a useful message describing the function of all the command line arguments.\n"
//                               "-l filename: Set the name of the log file. The default value for the name of the logfile is logfile.txt.\n"
//                               "-i y: determines how many times each slave should increment and write to\n"
//                               "the _le before terminating (default of 3)"
//                               "-s x: is the maximum number of slave processes spawned (default 5)"
//                               "-t z: the time in seconds when the master will terminate itself (default 20)\n\n");
//                break;
//            case 'i':
//                max_writes = optarg;
//                break;
//            case 'l':
//                filename = optarg;
//                break;
//            case 's':
//                if (optarg > 20)
//                {
//                    printf("Inputed: %d is to big. (Limit 20). Reverting back to default 5.\n", optarg);
//                    numOfSlaves = 5;
//                }
//                else
//                    numOfSlaves = optarg;
//                break;
//            case 't':
//                terminateTime = optarg;
//                break;
//            case '?':
//                if (isprint(optopt))
//                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
//                else
//                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
//                break;
//        }
//
//    // Let's say I want to pass some data to the slave (Hint you'll need to pass shmid for shareMemeory)
//    // For this Example I Will pass filename (You'll need to pass it) and lets pass your max_writes
//    // first your have to make anything that's not char a char
//    //since filename is a char we can ignore it
//    // Problem!!!! max_writes is a Int
//    // Let's make a char called passMax_writes
//    char passMax_writes[2]; //cool made
//    // now lets take max_writes and set passMax_writes to it
//    sprintf(passMax_writes, "%d'\0'", max_writes); // sprintf takes char , format of waht your trying to make char, and the value you want to convert
//    // Now we can pass max_writes as a char to our Slaves.
//    char pass_numOfSlaves[2];
//    sprintf(pass_numOfSlaves, "%d'\0'", numOfSlaves); // same thing with number of slaves
//
//    /**/
//    typedef struct PeterStruct // shared memory
//    {
//        int sum;
//        int turn;
//        enum state { idle, want_in, in_cs } flag[numOfSlaves];
//    }peter;
//
//    const int SHMSZ = sizeof(int);
//
//    if((shmid = shmget(key, SHMSZ, IPC_CREAT | 0600)) < 0)  // creating the shared memory
//    {
//        perror("shmget");
//        exit(1);
//    }
//    peter *shareData;
//    shareData = (peter*)shmat(shmid, NULL,0);
//    if (shareData->sum ==(int)-1) // Now we attach the segment to our data space.
//    {
//        perror("Shmat error in Main");
//        exit(1);
//    }
//    shareData->sum = 0;
//
//    int i;
//
//    /*
//     * We'll name our shared memory segment
//     * "0".
//     */
//
//    /*
//     * Create the segment.
//     */
////    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
////        perror("shmget");
////        exit(1);
////    }
////
////    /*
////     * Now we attach the segment to our data space.
////     */
////    if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1) {
////        perror("shmat");
////        exit(1);
////    }
//
//
//
//    /*
//     * Now put some things into the memory for the
//     * other process to read.
//     */
////    shm[0] = '0';
////    shm[1] = '1';
////    shm[2] = '\0';
//
//    char keyPass[32];
//    sprintf(keyPass, "%d",key);
//
//    char timePass[32];
//    sprintf(timePass, "%d", terminateTime);
//    /**/
//    char slaveNumber[8];
//    char shmidPass[8];
//    sprintf(shmidPass, "%d",shmid);
//
//    //Let's Start Forking
//    for (i = 0; i < numOfSlaves; i++) {
//        int sn = i + 1;
//        sprintf(slaveNumber, "%d'\0'", sn);
//        pid[i] = fork(); //forking
//        if (pid[i] < 0) //checking if fork failed
//        {
//            perror("fork failure");
//            exit(1);
//        }


void comOptions (int argc, char **argv, int c, int *x, int *y, int *z, char **filename, int *hflag, int *hepflag, int *nflag, int *lflag, int *nempty);
void displayHelpMesg();
void validate(int *x,int temp,char y);
void test(int x, int y, int z,char*file);  // this is just to print o
void CTRLhandler(int sig);  // handeling Ctrl^C signal
void on_alarm(int signal);  // handeling the alarm(z) signal

#define SHMSZ sizeof(int) //size
pid_t parent_pid;
int alarm_stop = 0;  // I don't know why I really need this but I have it
int z = 20; // the -t is global so on_alarm can access the variable without me having to pass it in
int x =5; // Also global
pid_t pidArr[20]; // Matt taught me how to be lazy and just 20 size array to store slave ID
int shmid;  //I made this global so CTRLhandler
int *sharedNum;

int main(int argc, char **argv)
{
    int c; //This is for the Switch statement for using getopt
    char *filename = "test.out";
    int y = 3;  // Times a slave should write to logfile
    int hflag =0;
    int hepflag =  0; // flags for each command for customized messages
    int nflag = 0;
    int lflag = 0;
    int nempty = 0;

    comOptions(argc,argv,c,&x,&y,&z,&filename, &hflag, &hepflag, &nflag, &lflag,&nempty); // H
    test(x,y,z,filename);
    key_t key = 1994; // making key for access
    typedef struct PeterStruct
    {
        int sum;
        int turn;
        enum state { idle, want_in, in_cs } flag[x];
    }peter;

    // Fork Proccesses

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
    shareData->sum = 0;


    char max_writes [32];
    sprintf(max_writes, "%d",y);
    char xx[32];
    //sprintf(xx, "%d",x);
    char keyPass[32];
    sprintf(keyPass, "%d",key);
    char shmidpass[32];
    sprintf(shmidpass, "%d",shmid);
    char numOfSlaves[32];
    sprintf(numOfSlaves, "%d",x);
    //char *args[] = {"./slave",filename,NULL};


    signal (SIGINT,CTRLhandler);
    signal(SIGALRM,on_alarm);
    alarm(z);
    int i = 0;
    for (i = 0; i < x ; i++)
    {
        sprintf(xx,"%d",i); // processes number
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Fork() Failed.");
            exit(-1);
        }
        if (pid == 0)
        {
            pidArr[x]=getpid();
            //printf("Child pid: %d\n",pidArr[x]);
            // printf("Starting Exec.\n");
            execl("./slave",filename,max_writes,xx,keyPass,shmidpass,numOfSlaves,NULL);
            signal (SIGINT,CTRLhandler);
            perror("Child failed to execl");
        }

    }



    wait(NULL);
    if((shmdt(shareData)) == -1) //detach from shared memory
    {
        perror("Error in shmdt in Parent:");
    }
    if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
    {
        perror ("Error in shmclt");
    }
    return 0;
}

//Functions----------------

void comOptions (int argc, char **argv , int c, int *x, int *y, int *z, char **filename, int *hflag, int *hepflag, int *nflag, int *lflag, int *nempty)
{
    int temp = 0;
    static struct option long_options[] =
            {
                    {"help", no_argument, 0, 'p'},
                    { 0,     0          , 0,  0 }
            };
    int long_index = 0;

    while((c = getopt_long_only(argc, argv, "hs:i:t:l:", long_options, &long_index)) != -1)
    {
        switch (c)
        {
            case 'h':  // -h
                *hflag = 1;
                *nempty = 1;
                if (*hepflag ==0) {displayHelpMesg();}
                break;

            case 'p':  // -help
                //printf("help used \n");
                *hepflag = 1;
                *nempty = 1;
                if (*hflag ==0) {displayHelpMesg();}
                break;

            case 's':  // -s x
                *nflag = 1;
                *nempty = 1;
                temp = *x;
                *x = atoi(optarg);
                if (*x > 20)
                {
                    printf("Inputed: %d is to big. (Limit 20). Reverting back to default 5.\n", *x);
                    *x = temp;
                }
                validate(x,temp,'x');
                break;

            case 'i':
                *nempty = 1;
                temp = *y;
                *y = atoi(optarg);
                validate(y,temp,'y');
                break;

            case 't':
                *nempty = 1;
                temp = *z;
                *z = atoi(optarg);
                validate(z,temp,'z');
                break;

            case 'l':
                if (optopt == 'n')
                {
                    printf("Please enter a valid filename.");
                    return;
                }
                //printf("Log file name changed to: %s\n", optarg);
                *lflag = 1;
                *nempty = 1;
                *filename = optarg;
                break;

            case '?':
                if (optopt == 'l')
                {
                    printf("Command -l requires filename. Ex: -lfilename.txt | -l filename.txt.\n");
                    exit(0);
                }
                else if (optopt == 's')
                {
                    printf("Commands -s requires int value. Ex: -s213| -s 2132\n");
                    exit(0);
                }
                else if (optopt == 'i')
                {
                    printf("Command -y requires int value. Ex: -i213| -i 2132\n");
                    exit(0);
                }
                else if (optopt == 't')
                {
                    printf("Command -z requires int value. Ex: -t13| -t 2132\n");
                    exit(0);
                }
                else
                {
                    printf("You have used an invalid command, please use -h or -help for command options, closing program.\n");
                    exit(0);
                }
                return;

            default :
                if (optopt == 'l')
                {
                    *lflag = 1;
                    *nempty = 1;
                    printf ("Please enter filename after -l \n");
                    exit(0);
                }
                else if (optopt == 'n')
                {
                    printf ("Please enter integer x after -n \n");
                    *nflag = 1;
                    *nempty = 1;
                }
                printf("Running Program without Commands.");
                break;
        }
    }
    if ((nempty == 0) &&argv[optind] == NULL || argv[optind + 1] == NULL)
    {
        printf("Running program without commands. Please use -h or -help for commands options next time.\n");
    }
}
void validate(int *x,int temp,char y)
{
    char *print;
    char *print2;
    if (y == 'y')
    {
        print = "y";
        print2 = "-i";
    }
    else if (y == 'z')
    {
        print = "z";
        print2 = "-t";
    }
    else if (y == 'x')
    {
        print = "x";
        print2 = "-s";
    }


    if (*x == 0)
    {
        printf("Intput invalid for %s changing %s back or default.\n",print2,print);
        *x = temp;
    }
    else if (*x < 0)
    {
        printf("Intput invalid for %s changing %s back or default.\n",print2,print);
        *x = temp;
    }
}
void displayHelpMesg()
{
    printf (" -h or -help  : shows steps on how to use the program \n");
    printf (" -s x         : x is the maximum number of slave processes spawned (default 5) \n");
    printf (" -l filename  : change the log file name \n");
    printf (" -i y         : The parameter y determines how many times each slave should increment and write to the file before terminating (default of 3). \n");
    printf (" -t z         : parameter z is the time in seconds when the master will terminate itself (default 20) \n");
    printf ("\nClosing Program Becuase it's a good idea, Good Luck Running the Program.\n");
    exit(0);
}
void test (int x, int y, int z, char *file)
{
    printf ("Number of Slaves (x): %d\n", x);
    printf ("Slaves will print(y): %d\n", y);
    printf ("Time limit       (z): %d\n", z);
    printf ("Filename            : %s\n\n", file);
}
void CTRLhandler(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nCtrl^C Called. Closing All Process.\n");
    fflush(stdout);

    if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
    {
        perror ("Error in shmclt");
    }
    int i =0;
    for (i=0; i<x;i++)
    {
        kill(pidArr[x], SIGQUIT);
    }


    exit(0);
}
void on_alarm(int signal)
{
        printf("Timer of %d seconds is over killing all slave processes.\n", z);
        int i = 0;
        for (i=0; i<x;i++)
        {
            kill(pidArr[x], SIGTERM); // killing em child by children
        }
        if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
        {
            perror ("Error in shmclt");
        }
        exit(0);
}



//
//        if (pid[i] == 0) //Fork didin't fail so executing slave
//        {
//            //Your Unix Book for OS exmplains all about Exec (there are many types)
//            // execl allows me to pass as many things as I like
//            // It takes in excutable and things you like to pass and NULL
//            execl("./slave", filename, passMax_writes, slaveNumber, pass_numOfSlaves, keyPass, timePass, shmidPass, (char*) NULL); //Your Doing it Your making Childrent (Slaves)
//            //slaveProcess(pid);
//            signal (SIGINT,CTRLhandler);
//            perror("Child failed to execl"); //precatuion tells you something happened (Check shared memory or zombie children if this happens)
//
//        }
//    }
//
//    /*
//     * Finally, we wait until the other process
//     * changes the first character of our memory
//     * to '*', indicating that it has read what
//     * we put there.
//     */
////    while (*shm != '*')
////        sleep(1);
//
//    // Detach segment
//    i = shmdt(shareData);
//    if(i == -1)
//    {
//        perror("shmop: shmdt failed");
//    } else
//    {
//        (void) fprintf(stderr, "shmop: shmdt returned %d\n", i);
//    }
//    if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
//    {
//        perror ("Error in shmclt");
//    }
//
//    return (0);
//}
//
//void CTRLhandler(int signum)
//{
//    signal(signum, SIG_IGN);
//    printf("\nCtrl^C Called. Closing All Process.\n");
//    fflush(stdout);
//
//    if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
//    {
//        perror ("Error in shmclt");
//    }
//    int i =0;
//    for (i=0; i<numOfSlaves;i++)
//    {
//        kill(pid[numOfSlaves], SIGQUIT);
//    }
//
//    exit(0);
//}
//
//void on_alarm(int signal)
//{
//        printf("Timer of %d seconds is over killing all slave processes.\n", terminateTime);
//        int i = 0;
//        for (i=0; i<numOfSlaves;i++)
//        {
//            kill(pid[numOfSlaves], SIGTERM); // killing em child by children
//        }
//        if((shmctl(shmid, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
//        {
//            perror ("Error in shmclt");
//        }
//        exit(0);
//}

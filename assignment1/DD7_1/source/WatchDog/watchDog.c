#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>


#define COUNTER_DIM 60

int counter = COUNTER_DIM; // counter with sleep(1) simulates a countdown of 60 seconds

// to have time and date for timestamps
time_t rawtime;
struct tm *timeinfo;

//log file 
FILE *log_watchDogFile;


//signal handler to manage signals from the other 4 processes
void signalHandler(int signo)
{
	if(signo == SIGUSR1)
	{
		time ( &rawtime );
		timeinfo = localtime(&rawtime);		
		fprintf(log_watchDogFile, " SIGNAL RECEIVED \t%s", asctime(timeinfo));
		fflush(log_watchDogFile);
		counter = COUNTER_DIM; // reset counter when a signal is received
	}
	
}

int main(int argc, char *argv[])
{
	// fds for sending motors' pids to the watchdog
	int fd_wdMx;
	int fd_wdMz;
	
	// fds for sending watchdog's pid to the other processes
	int fd_mxWd;
	int fd_mzWd;
	int fd_icWd;
	int fd_ccWd;
	
	// initializing pids
	pid_t pid_Mx, pid_Mz, pid_wd;
	
	// getting the watchdog's pid
	pid_wd = getpid();
	
	// motors' paths to the watchdog
	char * wdMx = "/tmp/wdMx";
	char * wdMz = "/tmp/wdMz";
	
	// watchdog's path to other processes
	char * mxWd = "/tmp/mxWd";
	char * mzWd = "/tmp/mzWd";
	char * icWd = "/tmp/icWd";
	char * ccWd = "/tmp/ccWd";
	
	// creating the named pipes
	mkfifo(wdMx, 0666);
	mkfifo(wdMz, 0666);	
	mkfifo(mxWd, 0666);
	mkfifo(mzWd, 0666);
	mkfifo(icWd, 0666);
	mkfifo(ccWd, 0666);
	
	// initializing the sigaction struct
	struct sigaction sa;
    	memset(&sa, 0, sizeof(sa));
    	sa.sa_handler = &signalHandler;    	
    	sigaction(SIGUSR1, &sa, NULL);
    	
	// opening the files descriptor	
	fd_wdMx = open(wdMx, O_RDONLY);
	fd_wdMz = open(wdMz, O_RDONLY);	
	fd_mxWd = open(mxWd, O_WRONLY);
	fd_mzWd = open(mzWd, O_WRONLY);
	fd_icWd = open(icWd, O_WRONLY);
	fd_ccWd = open(ccWd, O_WRONLY);
	
	// opening log file
	log_watchDogFile = fopen("./../logs/log_watchDogFile.txt", "w");
	
	//reading the pids
	read(fd_wdMx, &pid_Mx, sizeof(pid_Mx));  	
    	read(fd_wdMz, &pid_Mz, sizeof(pid_Mz));

    	// writing into the fds, to send wd's pid
    	write(fd_mxWd, &pid_wd, sizeof(pid_wd)+1);
    	write(fd_mzWd, &pid_wd, sizeof(pid_wd)+1);
    	write(fd_icWd, &pid_wd, sizeof(pid_wd)+1);
    	write(fd_ccWd, &pid_wd, sizeof(pid_wd)+1);
    	
    	while(1)
    	{
    		time ( &rawtime );
		timeinfo = localtime(&rawtime);
    		fprintf(log_watchDogFile, " Counter value is: %d \t%s", counter, asctime(timeinfo));
    		fflush(log_watchDogFile);
    		counter--; // 1 sec is passed
    		
    		if(counter == 0) // none process is active for 60 sec
    		{
    			// resetting the motors
    			kill(pid_Mx, SIGUSR2);
    			kill(pid_Mz, SIGUSR2);
    			counter = COUNTER_DIM; // resetting the countndown timer
    		}
    		sleep(1);
    	}
    	
	// closing fds
	close(fd_wdMx);
	close(fd_wdMz);	
	close(fd_mxWd);
	close(fd_mzWd);
	close(fd_icWd);
	close(fd_ccWd);
	
	//closing log file
	fclose(log_watchDogFile);
	
}

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

int main(int argc, char *argv[])

{

	int fd_icMx, fd_icMz; // fd for receiving mX and mZ pids
	int fd_icPosX; // fd for sending the position of the motorX to the inspection console
	int fd_icPosZ; // fd for sending the position of the motorZ to the inspection console
	int fd_icWd; // fd for sending wd pid to the ic
	
	char command; // variable for storing the command sent from the ic
		
	pid_t pid_Mx, pid_Mz, pid_wd; // initializing variables for storing pids
		
	int returnvalue; // variable for storing the return value of the select syscall
	
	// variables for storing motorX and motorZ positions
	float posX = 0.0;
	float posZ = 0.0;
	
	
	char * icMx = "/tmp/icMx"; // path of pipe for receiving mX's pid
	char * icMz = "/tmp/icMz"; // path of pipe for receiving mX's pid
	char * icPosX = "/tmp/icPosX"; // path of pipe for sending real time positions to the inspection console from motorX
	char * icPosZ = "/tmp/icPosZ"; // path of pipe for sending real time positions to the inspection console from motorX
	char * icWd = "/tmp/icWd"; // path of pipe for sending wd pid to the ic
	
	// Creating the named file(FIFO)
	mkfifo(icMx, 0666);
	mkfifo(icMz, 0666);	
	mkfifo(icPosX, 0666);
	mkfifo(icPosZ, 0666);	
	mkfifo(icWd, 0666);

	// opening the fds 
	fd_icMx = open(icMx, O_RDONLY);
    	fd_icMz = open(icMz, O_RDONLY);    	
    	fd_icPosX = open(icPosX, O_RDONLY);
    	fd_icPosZ = open(icPosZ, O_RDONLY);
    	fd_icWd = open(icWd, O_RDONLY);
    	
    	read(fd_icMx, &pid_Mx, sizeof(pid_Mx)); //reading mX's pid 
    	read(fd_icMz, &pid_Mz, sizeof(pid_Mz)); //reading mZ's pid   	
    	read(fd_icWd, &pid_wd, sizeof(pid_wd)); //reading wd's pid 
    	    	
    	fd_set set; //initialize the fd set
    	
	while(1)
	{
		// setting variables for using select between stdin and the inspection console
		struct timeval time;        
        	FD_ZERO(&set);
        	FD_SET(STDIN_FILENO, &set);
        	time.tv_sec = 0;
        	time.tv_usec = 495000;
        	returnvalue = select(STDIN_FILENO+1, &set, NULL, NULL, &time);	
	
		if(returnvalue == -1)
		{
			perror("select()");
		}
		else if(FD_ISSET(STDIN_FILENO, &set))
		{		
			read(STDIN_FILENO, &command, sizeof(command));
		}
	
		// switch for managing the different cases
		switch(command)
		{	
			// reset case
			case 'r': 
			case 'R':
			printf("\nRESET COMMAND INSPECTION CONSOLE\n");
			kill(pid_Mx, SIGINT);
			kill(pid_Mz, SIGINT);
			kill(pid_wd, SIGUSR1);
        		break;
	
			// stop case
		     	case 's':
		     	case 'S':
		     	printf("\nSTOP COMMAND INSPECTION CONSOLE\n");
			kill(pid_Mx, SIGTERM);
			kill(pid_Mz, SIGTERM);
			kill(pid_wd, SIGUSR1);             	
		     	break;
             	
		}
		// setting variables for using select between ic and the motors
		int select_fd;
		FD_ZERO(&set);
		FD_SET(fd_icPosX, &set);
		FD_SET(fd_icPosZ, &set);
		time.tv_sec = 0;
		time.tv_usec = 0;

        	if(fd_icPosX > fd_icPosZ)
        		select_fd = fd_icPosX;
        	else
        		select_fd = fd_icPosZ;
        		returnvalue = select(select_fd+1, &set, NULL, NULL, &time);
        	
        	if(returnvalue == -1)
        	{
        		perror("select()");
		}
       	
		if(FD_ISSET(fd_icPosX, &set))
		{       	
			read(fd_icPosX, &posX, sizeof(posX)+1);       			
		}       	
		if(FD_ISSET(fd_icPosZ, &set))
		{       	
			read(fd_icPosZ, &posZ, sizeof(posZ)+1);       		
		}  
		printf("Motor X position is: %f\t Motor Z position is: %f\n", posX, posZ);
        	
        	
	}
	
	// closing fds
	close(fd_icMx);
	close(fd_icMz);	
	close(fd_icPosX);
	close(fd_icPosZ);	
	close(fd_icWd);
	
	return 0;
}

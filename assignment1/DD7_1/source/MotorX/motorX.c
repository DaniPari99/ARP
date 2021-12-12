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

#define maxX 4.0
#define minX 0.0
#define step 0.1

// global variables for managing the motion of motor X
float posX = 0.0, virtualPosX = 0.0;
float estimatedPosX = 0.0, virtualEstimatedPosX = 0.0;

int command; // command received from command console
int fd_icPosX; // fd for sending the position of the motorX to the inspection console

// to have time and date for timestamps
time_t rawtime;
struct tm *timeinfo;

//log file 
FILE *log_mXFile;

// watchdog pid
pid_t pid_wd;

// function to generate random number
float randomFloat(float min, float max)
{
    time_t t;
    srand((unsigned) time(&t));

    float random = ((float) rand()) / (float) RAND_MAX;

    float range = max - min;  
    return (random*range) + min;
    
}


void signalHandler(int signo)
{
	if(signo == SIGUSR2 || signo == SIGINT)
	{	
		command = 7;		
	}
	else if(signo == SIGTERM)
	{
		
		time ( &rawtime );
		timeinfo = localtime(&rawtime);					
		fprintf(log_mXFile, " STOP SIGNAL RECEIVED \t%s", asctime(timeinfo));
		fflush(log_mXFile);
		
		time ( &rawtime );
		timeinfo = localtime(&rawtime);
          	fprintf(log_mXFile," The estimated position is: %f \t%s", estimatedPosX, asctime(timeinfo));
             	fflush(log_mXFile);
		
		command = 3;
	}	
}



int main(int argc, char *argv[])
{   
    
	int pid_Mx = getpid(); //pid motorX
    	int fd_ccMx; // fd for reading command from cc
    	int fd_icMx; // fd for sending/writing pid of the motorX for reset or stop    
    	int fd_wdMx; // fd for sending to wd the pid of the motorX
     	int fd_mxWd; // fd to get watchdog's pid 
    
    	// FIFO file path
    	char * ccMx = "/tmp/ccMx"; // path of pipe for sending/receiving command
    	char * icMx = "/tmp/icMx"; // path of pipe for sending/receiving pid of Mx
    	char * icPosX = "/tmp/icPosX"; // path of pipe for sending real time positions to the inspection console from motorX
    	char * wdMx = "/tmp/wdMx"; // path of pipe for sending the pid of the motorX to the wd
    	char * mxWd = "/tmp/mxWd"; // path per pid wd
    
    	// Creating the named file(FIFO)
    	mkfifo(ccMx, 0666);
    	mkfifo(icMx, 0666);
    	mkfifo(icPosX, 0666);
    	mkfifo(wdMx, 0666);    
    	mkfifo(mxWd, 0666);    

    	char str2[80]; // to store command received
    
    	
    	
    	//creating the struct of sigaction
    	struct sigaction sa;
    	memset(&sa, 0, sizeof(sa));
    	sa.sa_handler = &signalHandler;
    	sa.sa_flags = SA_RESTART;
    
    	sigaction(SIGINT, &sa, NULL);
    	sigaction(SIGTERM, &sa, NULL);
    	sigaction(SIGUSR2, &sa, NULL);
    
     	// opening the fds 
     	fd_ccMx = open(ccMx, O_RDONLY);
     	if(fd_ccMx == -1)
     	{
     		perror("open error");
     	} 
     	
     	fd_icMx = open(icMx, O_WRONLY);
     	if(fd_icMx == -1)
     	{
     		perror("open error");
     	}
        
     	fd_icPosX = open(icPosX, O_WRONLY);
     	if(fd_icPosX == -1)
     	{
     		perror("open error");
     	}
     	
     	fd_wdMx = open(wdMx, O_WRONLY);
     	if(fd_wdMx == -1)
     	{
     		perror("open error");
     	}
     	fd_mxWd = open(mxWd, O_RDONLY);
     	if(fd_mxWd == -1)
     	{
     		perror("open error");
     	}
     	
     	//opening log file
     	log_mXFile = fopen("./../logs/log_mXFile.txt", "w");
        
    	int returnvalue; // variable for storing the return value of the select syscall

    	write(fd_icMx, &pid_Mx, sizeof(pid_Mx)+1); //writing mz pid to the inspection console
    	write(fd_wdMx, &pid_Mx, sizeof(pid_Mx)+1); //writing mz pid to the watchdog
      	read(fd_mxWd, &pid_wd, sizeof(pid_wd)+1); //reading wd's pid 
    	
    	// initialize the fd set for using pipe select syscall
    	fd_set set;

    	while (1)
    	{
		// setting variables for using select
        	struct timeval time_select;        
        	FD_ZERO(&set);
        	FD_SET(fd_ccMx, &set);
        	time_select.tv_sec = 0;
        	time_select.tv_usec = 0;
        	returnvalue = select(fd_ccMx+1, &set, NULL, NULL, &time_select);

        	if(returnvalue == -1)
        	{
        		perror("select()");
        	}
        	else if (FD_ISSET(fd_ccMx, &set))
        	{
      			read(fd_ccMx, str2, 80);		
        		command = atoi(str2);       	
        	}
        	
                // switch for managing the different cases
        	switch(command)
            	{
            		// increasing X axis
               	case 1:
               	virtualPosX += step;
               	virtualEstimatedPosX = virtualPosX + randomFloat(-0.005, 0.005);
               	if(virtualEstimatedPosX > maxX)
               	{
               		estimatedPosX = maxX;
               		time ( &rawtime );
               		timeinfo = localtime(&rawtime);
               	 	fprintf(log_mXFile," The estimated position is: %f \t%s", estimatedPosX, asctime(timeinfo));
               	 	fflush(log_mXFile);
               	 	virtualPosX = maxX;
               	 	command = 3;
               	}
               	else 
               	{              	 
               		posX = virtualPosX;
               	 	estimatedPosX = posX + randomFloat(-0.005, 0.005);
               	 	time ( &rawtime );
               		timeinfo = localtime(&rawtime);
               	 	fprintf(log_mXFile," The estimated position is: %f \t%s", estimatedPosX, asctime(timeinfo));
               	 	fflush(log_mXFile);	 	
               	}
               	 
               	
               	write(fd_icPosX, &estimatedPosX, sizeof(estimatedPosX)+1); 
               	kill(pid_wd, SIGUSR1);	 
               	break;
	
			// decreasing X axis
               	case 2:
               	virtualPosX -= step;
               	virtualEstimatedPosX = virtualPosX + randomFloat(-0.005, 0.005);
               	if(virtualEstimatedPosX < minX)
               	{
               		estimatedPosX = minX;
               		time ( &rawtime );
               		timeinfo = localtime(&rawtime);
               	 	fprintf(log_mXFile," The estimated position is: %f \t%s", estimatedPosX, asctime(timeinfo));
               	 	fflush(log_mXFile);
               	 	virtualPosX = minX;
               	 	command = 3;
               	}
               	else 
               	{              	 
               		posX = virtualPosX;
               	 	estimatedPosX = posX + randomFloat(-0.005, 0.005);
               	 	time ( &rawtime );
               		timeinfo = localtime(&rawtime);
               	 	fprintf(log_mXFile," The estimated position is: %f \t%s", estimatedPosX, asctime(timeinfo));
               	 	fflush(log_mXFile);	 	
               	}
               	
               	write(fd_icPosX, &estimatedPosX, sizeof(estimatedPosX)+1); 
               	kill(pid_wd, SIGUSR1);
               	break;
	
			// stopping X axis
               	case 3:
               	
               	write(fd_icPosX, &estimatedPosX, sizeof(estimatedPosX)+1); 
               	break;
               	 
               	// managing case in which stop signal is called while resetting the motor
               	case 7:
               	time ( &rawtime );
               	timeinfo = localtime(&rawtime);
               	fprintf(log_mXFile, " RESET SIGNAL RECEIVED \t%s", asctime(timeinfo));
			fflush(log_mXFile);
		
			while(virtualPosX != minX)
			{	
				if(command == 3)
				break;
				
				virtualPosX -= step;
				virtualEstimatedPosX = virtualPosX + randomFloat(-0.005, 0.005);
				if(virtualEstimatedPosX < minX)
				{
					estimatedPosX = minX;
					time ( &rawtime );
               			timeinfo = localtime(&rawtime);
					fprintf(log_mXFile, " Returning to reset X position... %f meters still to do \t%s", estimatedPosX,  asctime(timeinfo));
					fflush(log_mXFile);
					virtualPosX = minX;
				}
				else
				{
					posX = virtualPosX;
					estimatedPosX = posX + randomFloat(-0.005, 0.005);
					time ( &rawtime );
               			timeinfo = localtime(&rawtime);
					fprintf(log_mXFile, " Returning to reset X position... %f meters still to do \t%s", estimatedPosX,  asctime(timeinfo));
					fflush(log_mXFile);
				}
			 	write(fd_icPosX, &estimatedPosX, sizeof(estimatedPosX)+1);	 
				kill(pid_wd, SIGUSR1); 
				usleep(500000);
			}
			command = 3;
               	break;

			default:

			break;
			
		}
                
            	usleep(500000);           
    	}
    	// closing fds
    	close(fd_ccMx);
    	close(fd_icMx); //pipe per passare il pid del mX
    	close(fd_icPosX);
    	close(fd_wdMx);
    	close(fd_mxWd);
    	
    	//closing log file
    	fclose(log_mXFile);
    	return 0;
}

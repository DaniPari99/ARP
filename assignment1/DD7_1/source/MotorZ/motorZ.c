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

#define maxZ 4.0
#define minZ 0.0
#define step 0.1

// global variables for managing the motion of motor Z
float posZ = 0.0, virtualPosZ = 0.0;
float estimatedPosZ = 0.0, virtualEstimatedPosZ = 0.0;

int command; // command received from command console
int fd_icPosZ; // fd for sending the position of the motorZ to the inspection console

// to have time and date for timestamps
time_t timeRaw;
struct tm *timeinfo;

//log file 
FILE *log_mZFile;

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

// signal handler
void signalHandler(int signo)
{
	if(signo == SIGINT || signo == SIGUSR2) //reset signal
	{	
		command = 8;		
	}
	else if(signo == SIGTERM) //stop signal
	{	
		time ( &timeRaw );
		timeinfo = localtime(&timeRaw);
		fprintf(log_mZFile, " STOP SIGNAL RECEIVED \t%s", asctime(timeinfo));
		fflush(log_mZFile);		
		time ( &timeRaw );
		timeinfo = localtime(&timeRaw);
          	fprintf(log_mZFile," The estimated position is: %f \t%s", estimatedPosZ, asctime(timeinfo));
             	fflush(log_mZFile);
             	
             	command = 6;
	}
}




int main(int argc, char *argv[])
{
    
	int pid_Mz = getpid(); //pid motorZ
    	int fd_ccMz; // fd for reading command from cc
    	int fd_icMz; // fd for sending/writing pid of the motorZ for reset or stop    
	int fd_wdMz; // fd to send Mz pid to the watchdog    
	int fd_mzWd; // fd to get watchdog's pid 
    

    	// FIFO file path
    	char * ccMz = "/tmp/ccMz"; // path of pipe for sending/receiving command
    	char * icMz = "/tmp/icMz"; // path of pipe for sending/receiving pid of Mz
    	char * icPosZ = "/tmp/icPosZ"; // path of pipe for sending real time positions to the inspection console from motorZ
    	char * wdMz = "/tmp/wdMz";// path of pipe for sending Mz pid to the watchdog
    	char * mzWd = "/tmp/mzWd"; // path for pid wd

    	// Creating the named file(FIFO)
    	mkfifo(ccMz, 0666);
    	mkfifo(icMz, 0666);
    	mkfifo(icPosZ, 0666);
    	mkfifo(wdMz, 0666);    
    	mkfifo(mzWd, 0666); 

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
    	fd_ccMz = open(ccMz, O_RDONLY);
       if(fd_ccMz == -1)
     	{
     		perror("open error");
     	}
     	      	
    	fd_icMz = open(icMz, O_WRONLY);
     	if(fd_icMz == -1)
     	{
     		perror("open error");
     	}
     	
    	fd_icPosZ = open(icPosZ, O_WRONLY); 
     	if(fd_icPosZ == -1)
     	{
     		perror("open error");
     	}
     	
     	fd_wdMz = open(wdMz, O_WRONLY); 
     	if(fd_wdMz == -1)
     	{
     		perror("open error");
     	}	
     
    	fd_mzWd = open(mzWd, O_RDONLY);
     	if(fd_mzWd == -1)
     	{
     		perror("open error");
     	}
     	
     	//opening log file
     	log_mZFile = fopen("./../logs/log_mZFile.txt", "w");
         
	int returnvalue; // variable for storing the return value of the select syscall
    	
    	write(fd_icMz, &pid_Mz, sizeof(pid_Mz)+1); //writing mz pid to the inspection console
    	write(fd_wdMz, &pid_Mz, sizeof(pid_Mz)+1); //writing mz pid to the watchdog
   	read(fd_mzWd, &pid_wd, sizeof(pid_wd)+1); //reading wd's pid    	
    	
	// initialize the fd set for using pipe select syscall
    	fd_set set;

    	while (1)
    	{
    		// setting variables for using select 
        	struct timeval time_selectMz;       
        	FD_ZERO(&set);
        	FD_SET(fd_ccMz, &set);
        	time_selectMz.tv_sec = 0;
        	time_selectMz.tv_usec = 0;
        	returnvalue = select(fd_ccMz+1, &set, NULL, NULL, &time_selectMz);
        
        	if(returnvalue == -1)
        	{
        		perror("select()");
        	}
        	else if (FD_ISSET(fd_ccMz, &set))
        	{
      			read(fd_ccMz, str2, 80);		
        		command = atoi(str2);       	
        	}
                // switch for managing the different cases  
        	switch(command)
		{
            		// increasing Z axis
               	case 4: 
               	virtualPosZ += step;
               	virtualEstimatedPosZ = virtualPosZ + randomFloat(-0.005, 0.005);
               	if(virtualEstimatedPosZ > maxZ)
               	{
               	 	estimatedPosZ = maxZ;
               	 	time ( &timeRaw );
               		timeinfo = localtime(&timeRaw);
               	 	fprintf(log_mZFile," The estimated position is: %f \t%s", estimatedPosZ, asctime(timeinfo));
               	 	fflush(log_mZFile);
               	 	virtualPosZ = maxZ;
               	 	command = 6;
               	}
               	else 
               	{              	 
               		posZ = virtualPosZ;
               	 	estimatedPosZ = posZ + randomFloat(-0.005, 0.005);
               	 	time ( &timeRaw );
               		timeinfo = localtime(&timeRaw);
               	 	fprintf(log_mZFile," The estimated position is: %f \t%s", estimatedPosZ, asctime(timeinfo));
               	 	fflush(log_mZFile);	 	
               	}
               	//printf("\nZ axis increase");
               	write(fd_icPosZ, &estimatedPosZ, sizeof(estimatedPosZ)+1);	 
               	kill(pid_wd, SIGUSR1);
               	break;
			 
			// decreasing Z axis
               	case 5:
               	virtualPosZ -= step;
               	virtualEstimatedPosZ = virtualPosZ + randomFloat(-0.005, 0.005);
               	if(virtualEstimatedPosZ < minZ)
               	{
               	 	estimatedPosZ = minZ;
               	 	time ( &timeRaw );
               		timeinfo = localtime(&timeRaw);
               	 	fprintf(log_mZFile," The estimated position is: %f \t%s", estimatedPosZ, asctime(timeinfo));
               	 	fflush(log_mZFile);
               	 	virtualPosZ = minZ;
               	 	command = 6;
               	}
               	else 
               	{              	 
               		posZ = virtualPosZ;
               	 	estimatedPosZ = posZ + randomFloat(-0.005, 0.005);
               	 	time ( &timeRaw );
               		timeinfo = localtime(&timeRaw);
               	 	fprintf(log_mZFile," The estimated position is: %f \t%s", estimatedPosZ, asctime(timeinfo));
               	 	fflush(log_mZFile); 	 	
               	}
               	//printf("\nZ axis decrease");
               	write(fd_icPosZ, &estimatedPosZ, sizeof(estimatedPosZ)+1);
               	kill(pid_wd, SIGUSR1);
               	break;
				
			// stopping Z axis
               	case 6:
               	
               	write(fd_icPosZ, &estimatedPosZ, sizeof(estimatedPosZ)+1);
               	break;
               	 
               	// managing case in which stop signal is called while resetting the motor
               	case 8:               	 
               	time ( &timeRaw );
               	timeinfo = localtime(&timeRaw);
               	fprintf(log_mZFile, " RESET SIGNAL RECEIVED \t%s", asctime(timeinfo));
		
			while(virtualPosZ != minZ)
			{	
				if(command == 6)
					break;
			 	virtualPosZ -= step;
               	 	virtualEstimatedPosZ = virtualPosZ + randomFloat(-0.005, 0.005);
               	 	if(virtualEstimatedPosZ < minZ)
               	 	{
               	 		estimatedPosZ = minZ;
               	 		time ( &timeRaw );
               			timeinfo = localtime(&timeRaw);
					fprintf(log_mZFile, " Returning to reset Z position... %f meters still to do \t%s", estimatedPosZ,  asctime(timeinfo));
					fflush(log_mZFile);
               	 		virtualPosZ = minZ;
               	 	}
               		else 
               	 	{              	 
               			posZ = virtualPosZ;
               	 		estimatedPosZ = posZ + randomFloat(-0.005, 0.005);
               	 		time ( &timeRaw );
               			timeinfo = localtime(&timeRaw);
					fprintf(log_mZFile, " Returning to reset Z position... %f meters still to do \t%s", estimatedPosZ,  asctime(timeinfo));
					fflush(log_mZFile);	 	
               	 	}
               	 	write(fd_icPosZ, &estimatedPosZ, sizeof(estimatedPosZ)+1);	 
               	 	kill(pid_wd, SIGUSR1);
               	 	usleep(500000);
			}
			
			command = 6; 
               	 
               	break;

			default:
			
			
			break;
			
                
		}
      		usleep(500000);     
            
	}
	
    	// closing fds
    	close(fd_ccMz);
    	close(fd_icMz);
    	close(fd_icPosZ);
    	close(fd_wdMz);
    	close(fd_mzWd);
    	
    	//closing log file
    	fclose(log_mZFile);
    	return 0;
}

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

//functions to colour the printf
void yellow()
{
        printf("\033[1;33m");
}
void reset()
{
        printf("\033[0m");
}
void blue()
{
        printf("\033[1;34m");
}
void red()
{
        printf("\033[1;31m");
}



int main()
{
	pid_t pid_wd; // variable for storing wd's pid
    
    	int fd_ccMx; // fd for sending the command from the cc to the mX
    	int fd_ccMz; // fd for sending the command from the cc to the mZ
    	int fd_ccWd; // fd for sending wd pid to cc
  
    	char * ccMx = "/tmp/ccMx"; // path of pipe for sending/receiving command
    	char * ccMz = "/tmp/ccMz"; //path of pipe for sending/receiving command    
    	char * ccWd = "/tmp/ccWd"; // path for sending wd pid to cc

    	// Creating the named file(FIFO)
    	mkfifo(ccMx, 0666);
    	mkfifo(ccMz, 0666);    
    	mkfifo(ccWd, 0666);
    
    	char arrCommand[80];
    
    	int val = 0;
    
	// opening the fds 
    	fd_ccMx = open(ccMx, O_WRONLY);
    	fd_ccMz = open(ccMz, O_WRONLY);  //pipe per passare il valore al motorZ    
    	fd_ccWd = open(ccWd, O_RDONLY);
    
    	// read wd's pid
    	read(fd_ccWd, &pid_wd, sizeof(pid_wd));
    
    	while (1)
	{
    	
		blue();
        	printf("\n\n--------Welcome in the Command Console--------\n\nYou can:");         	
        	printf(" \n\n 1) Type 1 for X axis increase\n 2) Type 2 for X axis decrease\n 3) Type 3 for X axis stop"); 
        	printf("\n 4) Type 4 for Z axis increase\n 5) Type 5 for Z axis decrease\n 6) Type 6 for Z axis stop\n ");
        	yellow();
        	printf("\nIf you want to reset or stop the motors you have to type in the Inspection Console:\n\n 1) r to reset the hoist position\n 2) s to stop the hoist \n");
        	blue();
        	printf("\nInsert the command:");
        	fflush(stdout);
        	fgets(arrCommand, 80, stdin); 
        	reset();

        	val = atoi(arrCommand);
        	
        	// 'if' for checking the correctness of the button pressed from the user
        	if(val == 1 || val == 2 || val == 3 || val == 4 || val == 5 || val == 6)
        	{											
        		if (val > 0 && val < 4) // To pass value to motorX
        		{	
        			write(fd_ccMx, arrCommand, strlen(arrCommand)+1); 				  
        		}
        
        		if (val > 3 && val < 7) // To pass value to motorZ
        		{
        			write(fd_ccMz, arrCommand, strlen(arrCommand)+1);         					      
        		}
        		kill(pid_wd, SIGUSR1);
	
        	}
        	
        	// whether the button pressed is wrong
		else
        	{
        		red();
        		printf("\n\tINVALID COMMAND!!! \n\tRetype: \n");
        		fflush(stdout);
             	} 
	}
    
	// closing fds
    	close(fd_ccMx);
    	close(fd_ccMz);    
    	close(fd_ccWd);    
    	return 0;
}

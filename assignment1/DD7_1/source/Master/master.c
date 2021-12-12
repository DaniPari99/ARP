#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>


#define pipe_number 13

//FUNCTION THAT IS CALLED TO FORK AND EXEC A PROCESS 
int spawn(const char * program, char ** arg_list) 
{
  	pid_t child_pid = fork();
  	
  	if (child_pid != 0)
    	return child_pid;
  	else 
  	{
    		execvp (program, arg_list);
    		perror("exec failed");
    		return 1;
  	}
}

int main() {
  
	char* pipe[pipe_number];

  
 	//path initialization of pipes
 
 	pipe[0] = "/tmp/ccMx"; //pipe for sending command from the command console to the Mx
 	pipe[1] = "/tmp/ccMz"; //pipe for sending command from the command console to the Mz
 	pipe[2] = "/tmp/icMx"; //pipe for sending Mx's pid to the ic
 	pipe[3] = "/tmp/icMz"; //pipe for sending Mx's pid to the ic
 	pipe[4] = "/tmp/icPosX"; //pipe for sending mX position to the inspection console
 	pipe[5] = "/tmp/icPosZ"; //pipe for sending mZ position to the inspection console
 	pipe[6] = "/tmp/wdMx"; //for sending mX's pid to the wd
 	pipe[7] = "/tmp/wdMz"; //for sending mZ's pid to the wd
 	pipe[8] = "/tmp/mxWd"; //for sending wd's pid to the mx
 	pipe[9] = "/tmp/mzWd"; //for sending wd's pid to the mz
 	pipe[10] = "/tmp/icWd"; //for sending wd's pid to the ic
 	pipe[11] = "/tmp/ccWd"; //for sending wd's pid to the cc
 
	//ARRAY FOR OPENING KONSOLE AND PROCESSES
	char * arg_list_1[] = { "/usr/bin/konsole",  "-e", "./../CommandConsole/commandConsole", pipe[0], pipe[1], pipe[11], (char*)NULL }; 
	char * arg_list_2[] = { "/usr/bin/konsole",  "-e", "./../InspectionConsole/inspectionConsole", pipe[2], pipe[3], pipe[4], pipe[5], pipe[10], (char*)NULL };
  	char * arg_list_3[] = { "./../MotorX/motorX", pipe[0] , pipe[4], pipe[6], pipe[8], (char*)NULL }; 
	char * arg_list_4[] = { "./../MotorZ/motorZ",pipe[1], pipe[5], pipe[7], pipe[9], (char*)NULL }; 
	char * arg_list_5[] = { "./../WatchDog/watchDog",pipe[6], pipe[7], pipe[8], pipe[9], pipe[10], pipe[11], (char*)NULL  }; 
	
	//INITIALIZING KONSOLE'S PID
	int pidCc;
	int pidIc;
	int pidMx;
	int pidMz;
	int pidWd;

  	//CREATE THE NAMED PIPE AND CANCEL IF THEY ALREADY EXIST  
  	if( access( "/tmp/ccMx", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/ccMx");
  	}
  	if( access( "/tmp/ccMz", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/ccMz");
  	}
  	if( access( "/tmp/icCc", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/icCc");
  	}
  	if( access( "/tmp/icMx", F_OK ) != -1 )
  	{ 
    		unlink("/tmp/icMx");
  	}
  	if( access( "/tmp/icMz", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/icMz");
  	}
  	if( access( "/tmp/icPosX", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/icPosX");
  	}
  	if( access( "/tmp/icPosZ", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/icPosZ");
  	}
  	if( access( "/tmp/wdMx", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/wdMx");
  	}
  	if( access( "/tmp/wdMz", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/wdMz");
  	}
  	if( access( "/tmp/mxWd", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/mxWd");
  	}
  	if( access( "/tmp/mzWd", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/mzWd");
  	}
  	if( access( "/tmp/icWd", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/icWd");
  	}
  	if( access( "/tmp/ccWd", F_OK ) != -1 ) 
  	{ 
    		unlink("/tmp/ccWd");
  	}
  		

  	//RUN THE PROCESSES DIFFERENT KONSOLES 
  	pidCc = spawn("/usr/bin/konsole", arg_list_1);

  	pidIc = spawn("/usr/bin/konsole", arg_list_2);

  	pidMx = spawn("./../MotorX/motorX", arg_list_3);

  	pidMz = spawn("./../MotorZ/motorZ", arg_list_4);

  	pidWd = spawn("./../WatchDog/watchDog", arg_list_5);

  	return 0;
}


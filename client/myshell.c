#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>

#define _GNU_SOURCE
#define BUFFSIZE 4096

void myShellWait(pid_t waitPid); // executes wait on the waitPid. -1 waits for arbitrary program
pid_t myShellStart(char *args[]); // starts the program specifies, does not wait
int myShellSignal(pid_t killPid, int signal); // sends signal to pid killPid
char isInteger(char *number); // checks if a string can be converted to a valid integer

int main(int argc, char *argv[]) {

	char line[BUFFSIZE]; // declare a line variable of max size BUFFSIZE
	char *words[101]; // create a words araay to store the arguments
	int i;
	
	printf("myshell> ");
	fflush(stdout);
	while(fgets(line, BUFFSIZE, stdin)) { // loop until the input is finished
		size_t len = strlen(line);
		if (len > 0 && line[len-1] == '\n') { // remove the newline from the end of the line
			line[--len] = '\0';
		}
			
		int numWords = 0;
		words[numWords] = strtok(line," ");
		while(numWords < 100 && words[numWords] != NULL) { // read in words from the line into the words variable
			numWords++;
			words[numWords] = strtok(NULL, " ");
		}
		words[numWords] = NULL; // set the next cell to NULL	
		
		char *commandToExecute[100];
		
		if(words[0] == NULL) { // check that a command was put in
			printf("myshell> ");
			fflush(stdout);
			continue;
		}

		for(i = 0; words[i+1] != NULL && i < 99; i++) { // separate the myshell command from the rest
			commandToExecute[i] = words[i+1];
		}
		commandToExecute[i] = NULL;	
		
		if(strcmp(words[0], "start") == 0) { // check if the user ran "start"
			myShellStart(commandToExecute);

		} else if(strcmp(words[0], "run") == 0) { // check if the user ran "run"
			pid_t childPid = myShellStart(commandToExecute);
			myShellWait(childPid);

		} else if(strcmp(words[0], "wait") == 0) { // check if the user ran "wait"
			myShellWait(-1);

		} else if(strcmp(words[0], "stop") == 0) { // check if the user ran "stop"
			if(isInteger(commandToExecute[0])) { // make sure the pid passed in is an integer
				int failure = myShellSignal(atoi(commandToExecute[0]), SIGSTOP); // signal the specified pid with sigstop
				if(!failure) {
					printf("myshell: process %d paused successfully\n", atoi(commandToExecute[0]));
				}
			} else {
				printf("myshell: unable to stop: please input an integer pid\n");
			}
				
		} else if(strcmp(words[0], "continue") == 0) { // check if the user ran "continue"
			if(isInteger(commandToExecute[0])) { // make sure the pid passed in is an integer
				int failure = myShellSignal(atoi(commandToExecute[0]), SIGCONT); // signal the specified pid with sigcont
				if(!failure) {
					printf("myshell: process %d continued successfully\n", atoi(commandToExecute[0]));
				}
			} else {
				printf("myshell: unable to continue: please input an integer pid\n");
			}

		}  else if(strcmp(words[0], "kill") == 0) { // check if the user ran "kill"
			if(isInteger(commandToExecute[0])) { // make sure the pid passed in is an integer
				int failure = myShellSignal(atoi(commandToExecute[0]), SIGKILL); // signal the specified pid with sigkill
				if(!failure) {
					printf("myshell: process %d killed successfully\n", atoi(commandToExecute[0]));
				}
			} else {
				printf("myshell: unable to kill: please input an integer pid\n");
			}

		} else if(strcmp(words[0], "exit") == 0 || strcmp(words[0], "quit") == 0) { // check if the user ran "quit" or "exit"
			_exit(0);
		} else {
			printf("Unknown command: %s\n", words[0]);
		}	


		printf("myshell> ");
		fflush(stdout);
	}
	
	return 0;
}

int myShellSignal(pid_t killPid, int signal) { // signals killPid with signal
	int success = kill(killPid, signal);
	return 0;
	if(success == -1) { // handle errors
		printf("myshell: unable to kill: %s\n", strerror(errno));
		return errno;
	}
}

void myShellWait(pid_t waitPid) { // wait for process with pid waitPid. -1 waits for arbitrary program
	int status;
	pid_t pid = waitpid(waitPid, &status, 0); // wait
	if(pid < 0) { // check errors
		printf("myshell: unable to wait, %s\n", strerror(errno));
	} else { // collect status
		if(WIFEXITED(status)) { 
			int r = WEXITSTATUS(status);
			printf("Process %d exited normally with exit status %d\n", pid, r);
		} else if(WIFSIGNALED(status)) {
			int r = WTERMSIG(status);
			printf("Process %d exited abnormally with exit signal %d: %s\n", pid, r, strsignal(r));;
		}
	}
}

pid_t myShellStart(char *args[]) { // fork a new program specified by args, no wait

	pid_t pid = fork(); // fork

	if(pid < 0) { // check errors
		printf("myshell: unable to fork, %s\n", strerror(errno));
	} else if(pid > 0) { // in parent, relay that process started
		printf("myshell: process %d started\n", pid);
		return pid;
	} else if(pid == 0) { // in child, exec
		execvp(args[0], args);
		printf("myshell: unable to exec %s, %s\n", args[0], strerror(errno));
		_exit(errno);
	}

	return pid;
}

// got this function from online. Checks whether a string can be converted to a valid integer
char isInteger(char *number) {
	int i = 0;
	if (number[0] == '-') { i = 1; }
	for (; number[i] != 0; i ++) {
		if(!isdigit(number[i])) return 0;
	}
	return 1;
}

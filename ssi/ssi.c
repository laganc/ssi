#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Initialize buffers for prompt
char* user_name;
char host_name[1024];
char prompt_buffer[1024];
char cwd_buffer[1024];

// Linked list struct for background processes
typedef struct bg_pro{
	pid_t pid;
	char command[1024];
	struct bg_pro* next;
} bg_pro;

// Initializing head of list
bg_pro* head = NULL;

// Adding a new process to list
// pid: Process ID of process being added
// comm: Command of process
void addProcess(pid_t pid, char comm[]){
	// Initialize new node
	bg_pro* new_pro = (bg_pro*)malloc(sizeof(bg_pro));
	new_pro->pid = pid;
	strcat(new_pro->command, comm);
	new_pro->next = NULL;

	if(head == NULL){
		head = new_pro;
	}
	else{
		bg_pro* temp = head;
		while(temp->next != NULL){
			temp = temp->next;
		}
		temp->next = new_pro;
	}
}

// Print processes when 'bglist' is called
void printProcesses(){
	int sum = 0;
	bg_pro* temp = head;
	while(temp != NULL){
		printf("%d: %s\n", temp->pid, temp->command);
		temp = temp->next;
		sum++;
	}
	printf("Total Background Jobs: %d\n", sum);
}

// Create prompt for user input
char* get_prompt(){
	user_name = getlogin();
	gethostname(host_name, 1024);
	getcwd(cwd_buffer, 1024);
	snprintf(prompt_buffer, 1024, "%s@%s: %s> ", user_name, host_name, cwd_buffer);
	char* prompt = prompt_buffer;
	return prompt;
}

// Split up user input into arguments
// args: Buffer to place the arguments from input
// input: User input
void tokenize(char* args[1024], char* input){
	args[0]=strtok(input," \n");//split string by space or '\n' (IMPORTANT) there will be error without '\n'
  int i=0;
  
  while(args[i]!=NULL){//make sure that args has a NULL pointer at the end
    args[i+1]=strtok(NULL," \n");
    i++;
	}
}

// Create new processes
// args: The arguments from user input
// background: If 0, it is a foreground process. If 1, it is a background process
void fork_process(char* args[1024], int background){
	pid_t p = fork();
	int status;

	switch(background){

		// Foreground Process
		case 0:
			// Child
			if(p == 0){
				execvp(args[0], args);
				printf("Unable to execute.\n");
				exit(1);
			}
			// Parent
			else if(p > 0){
				pid_t w = waitpid(p, &status, 0);
			}
			// Fork error
			else{
				printf("Error.\n");
				exit(-1);
			}
			break;

		// Background Process
		case 1:
			// Child
			if(p == 0){
				execvp(args[0], args);
				printf("Unable to execute.\n");
				exit(1);
			}
			// Parent
			else if(p > 0){
				pid_t w = waitpid(p, &status, WNOHANG);
				char comm[1024] = "";
				int i = 0;
				strcat(comm, cwd_buffer);
				strcat(comm, "/");
				while(args[i] != NULL){
					strcat(comm, args[i]);
					strcat(comm," ");
					i++;
				}
				comm[strlen(comm)-1] = '\0';
				addProcess(p, comm);
			}
			// Fork error
			else{
				printf("Error.\n");
				exit(-1);
			}
			break;
	}
}

// Change directory (cd command)
// args: The arguments from user input
void change_directory(char* args[1024]){
	if(strcmp(args[1],"~") == 0){
		chdir(getenv("HOME"));
	}
	else{
		int cdv = chdir(args[1]);
		if(cdv != 0){
			printf("\'%s\' does not exist.\n", args[1]);
		}
	}
}

// Check to see if process is finished
void checkTermination(){
	if(head == NULL){
		return;
	}

	if(head != NULL){
		pid_t ter = waitpid(0, NULL, WNOHANG);
		if(ter > 0){
			if(head->pid == ter){
				printf("%d has been terminated.\n", head->pid);
				head = head->next;
			}
			else{
				bg_pro* temp = head;
				while(temp->next != NULL){
					if(temp->pid == ter){
						printf("%d has been terminated.\n", temp->pid);
						temp->next = temp->next->next;
					}
					temp = temp->next;
				}
			}
		}
	}
}


int main() {
	char* prompt = get_prompt();
	int bailout = 0;

	while (!bailout){
		prompt = get_prompt();
		char* input = readline(prompt);

		// Checks if any processes are finished upon user input
		checkTermination();
		
		// Exit
		if (strcmp(input, "exit") == 0){
			bailout = 1;
		} 

		// If input is empty, prompt again
		else if (strcmp(input, "") == 0){
		} 

		// Change directory to home
		else if(strcmp(input, "cd") == 0){
			chdir(getenv("HOME"));
		}

		// Display all background processes
		else if(strcmp(input, "bglist") == 0){
			printProcesses();
		}

		else {
			char* args[1024];
			tokenize(args, input);

			// Change directory
			if(strcmp(args[0], "cd") == 0){
				change_directory(args);
			}

			// Execute background process
			else if(strcmp(args[0], "bg") == 0){
				for(int i = 0; i <= strlen(*args); i++){
					args[i] = args[i+1];
				}
				fork_process(args, 1);
			}

			// Execute foreground process
			else{
				fork_process(args, 0);
			}
		}
		free(input);
	}
	printf("Exiting.\n");
}

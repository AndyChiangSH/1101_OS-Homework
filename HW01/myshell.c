/*
OS homework01 - myshell source code
author: AndyChiang
time: 2021/12/01
line: 256
Don't copy without permission!
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <readline/readline.h>
#include <readline/history.h>
#define SSLEN 100

FILE * fout;
FILE * fhist;
char buffer[SSLEN];

// handler wait for exit of child process
void SIGCHLD_handler()
{
	pid_t pid;
    int status;

    while((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

// get max in two number
int max(int a, int b) {
	if(a > b) {
		return a;
	}
	return b;
}

// check whether str is a number(only digit)
int isNumber(char* str) {
	int i;
	for(i = 0; i < strlen(str); i++) {
		if(isdigit(str[i]) == 0) {
			return 0;
		}
	}
	return 1;
}

// print initial prompt
void shell_init() {
	printf("\033[0;36m");
	printf("\n------------------------------------------------\n\n");
	printf("        Hi, %s. Welcome to myshell!\n\n", getenv("USER"));
	printf("   You can use 'help' to get more information\n");
	printf("                about myshell.\n\n");
	printf("      And... have fun with myshell! (ﾉ>ω<)ﾉ\n\n");
	printf("         copyright @ 2021 AndyChiang\n\n");
	printf("------------------------------------------------\n\n");
	printf("\033[0;0m");
	return;
}

int main(int argc, char **argv) {
	int i;
	char* ps;
	char ss[SSLEN];
	char* ss_ptr = &ss[0];
	char ss2[SSLEN];
	char* ss2_ptr = &ss2[0];
	
	signal(SIGCHLD, SIGCHLD_handler);	// set signal handler
	using_history();
	read_history(".history_file");	// import history from .history
	shell_init();
	while(1) {
		// enter command
		char* username = getenv("USER");
		sprintf(ps, "\033[0;32m%s\033[0;0m@\033[0;33m%s\033[0;0m: ", username, getcwd(buffer, sizeof(buffer)));
		ss_ptr = readline(ps);
		if(ss_ptr == NULL) {	// exit myshell
			fflush(stdout);
			write_history(".history_file");
			printf("Bye Bye!\n");
			exit(0);
		}
		else if(strlen(ss_ptr) == 0) {
			continue;
		}
		
		strcpy(ss2_ptr, ss_ptr);

		// count numbers of arguments and modes
		char* arg = NULL;
		char* fout_name = "\0";
		int args_num = 0, read_mode = 0, bg_mode = 0, output_mode = 0;
		
		// split ss by space
		arg = strtok(ss_ptr, " ");
		if(arg == NULL) {	// ss only space
			continue;
		}
		add_history(ss2_ptr);	// add into history buffer
		
		while(arg != NULL) {
			if(strcmp(arg, ">") == 0) {	// file redirection: overwrite
				read_mode = 1;
				output_mode = 1;
				fout_name = strtok(NULL, " ");
			}
			else if(strcmp(arg, ">>") == 0) {	// file redirection: append
				read_mode = 1;
				output_mode = 2;
				fout_name = strtok(NULL, " ");
			}
			else if(strcmp(arg, "&") == 0) {	// background execution
				read_mode = 1;
				bg_mode = 1;
			}
			if(read_mode == 0) {
				args_num++;
			}
			arg = strtok(NULL, " ");
		}
		
		// output redirection: overwrite or append
		int fn, copy;
		if(output_mode > 0) {
			if(output_mode == 1) {	// overwrite
				fout = fopen(fout_name, "w");
			}
			else if(output_mode == 2) {		// append
				fout = fopen(fout_name, "a");
			}
			fn = fileno(fout);
			copy = dup(1);
			dup2(fn, 1);	// change stdout to file
		}
		
		// split argument by space
		char* args[args_num+1];
		char arg_cat[] = "";
		arg = strtok(ss2_ptr, " ");
		args[0] = arg;
		for(i = 1; i < args_num; i++) {
			arg = strtok(NULL, " ");
			args[i] = arg;
			strcat(arg_cat, arg);	// concatenate arguments that before redirection or background
			if(i != args_num-1) {
				strcat(arg_cat, " ");
			}
		}
		args[args_num] = NULL;
		
		// case of command
		if(strcmp(args[0], "cd") == 0) {
			chdir(args[1]);
		}
		else if(strcmp(args[0], "pwd") == 0) {
			printf("%s\n", buffer);
		}
		else if(strcmp(args[0], "echo") == 0) {
			printf("%s\n", arg_cat);
		}
		else if(strcmp(args[0], "export") == 0) {
			setenv("PATH", arg_cat, 1);
			printf("PATH: %s\n", getenv("PATH"));
		}
		else if(strcmp(args[0], "help") == 0) {
			printf("\n----------Welcom to myshell help page----------\n\n");
			printf("We support these command below:\n");
			printf("cd: cd [dir_path]\n");
			printf("\tMove to other directory.\n");
			printf("pwd: pwd\n");
			printf("\tPrint current working directory.\n");
			printf("echo: echo [text]\n");
			printf("\tPrint Print text.\n");
			printf("export: export [PATH_variable]\n");
			printf("\tSet PATH variable.\n");
			printf("history: history [-clear] [number]\n");
			printf("\tPrint history command.\n");
			printf("\t[number]: Print top number history command.\n");
			printf("\t[-clear]: Clear history.\n");
			printf("help: help\n");
			printf("\tPrint help page.\n");
			printf("External command: like ls, cp, rm...\n");
			printf("We also support background execution (&) and output redirection (>, >>)\n");
			printf("<Ctrl+d>: Exit myshell.\n\n");
			printf("------------------------------------------------\n\n");
		}
		else if(strcmp(args[0], "history") == 0) {
			// get history buffer
			HIST_ENTRY **hist_list = history_list();
			if(hist_list) {
				printf("Total history in buffer: %d\n", history_length);
				if(args_num == 1) {	// all history
					for(i = 0; hist_list[i]; i++) {
						printf("%4d: %s\n", i+history_base, hist_list[i]->line);
					}
				}
				else {
					if(isNumber(args[1])) {	// top number history
						int hist_offset = atoi(args[1]);
						for(i = max(history_length-hist_offset, 0); i < history_length; i++) {
							printf("%4d: %s\n", i+history_base, hist_list[i]->line);
						}					
					}
					else {
						if(strcmp(args[1], "-clear") == 0) {	// clear history
							clear_history();
							fhist = fopen(".history_file", "w");
							fclose(fhist);
						}
						else {
							printf("[ERROR] history: Invalid operand '%s'.\n", args[1]);
						}
					}
				}
			}
			else {
				printf("[ERROR] No history now!\n");
			}
			
		}
		else {	// external command
			pid_t pid;
			pid = fork();
			if(pid < 0) {
				printf("[ERROR] Fork failed.\n");
				exit(-1);
			}
			else if (pid == 0) {	// child process
				if(execvp(args[0], args) == -1) {
					printf("[ERROR] Command '%s' not found.\n", args[0]);
				}
				exit(0);
			}
			else {		// parent process
				if(bg_mode == 0) {	// not background mode
					wait(NULL);
				}
			}
		}
		
		// clear stdout buffer
		fflush(stdout);
		// recover stdout
		if(output_mode == 1 || output_mode == 2) {
			dup2(copy, 1);
			fclose(fout);
		}
	}
}

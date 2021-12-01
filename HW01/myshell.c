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

void SIGCHLD_handler()
{
	pid_t pid;
    int status;

    while((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

int max(int a, int b) {
	if(a > b) {
		return a;
	}
	return b;
}

int isNumber(char* str) {
	int i;
	for(i = 0; i < strlen(str); i++) {
		if(isdigit(str[i]) == 0) {
			return 0;
		}
	}
	return 1;
}

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
	
	signal(SIGCHLD, SIGCHLD_handler);
	using_history();
	read_history(".history_file");
	shell_init();
	while(1) {
		char* username = getenv("USER");
		sprintf(ps, "\033[0;32m%s\033[0;0m@\033[0;33m%s\033[0;0m: ", username, getcwd(buffer, sizeof(buffer)));
		ss_ptr = readline(ps);
		if(ss_ptr == NULL) {
			fflush(stdout);
			write_history(".history_file");
			printf("Bye Bye!\n");
			exit(0);
		}
		else if(strlen(ss_ptr) == 0) {
			continue;
		}
		add_history(ss_ptr);
		// printf("ss: %s\n", ss_ptr);
		
		strcpy(ss2_ptr, ss_ptr);

		// count numbers of arguments and modes
		char* arg = NULL;
		char* fout_name = "\0";
		int args_num = 0, read_mode = 0, bg_mode = 0, output_mode = 0;
		arg = strtok(ss_ptr, " ");
		while(arg != NULL) {
			// printf("%s\n", arg);
			if(strcmp(arg, ">") == 0) {
				read_mode = 1;
				output_mode = 1;
				fout_name = strtok(NULL, " ");
			}
			else if(strcmp(arg, ">>") == 0) {
				read_mode = 1;
				output_mode = 2;
				fout_name = strtok(NULL, " ");
			}
			else if(strcmp(arg, "&") == 0) {
				read_mode = 1;
				bg_mode = 1;
			}
			if(read_mode == 0) {
				args_num++;
			}
			arg = strtok(NULL, " ");
		}
		// printf("arg_num: %d\n", arg_num);
		// printf("args_num = %d, bg_mode = %d, output_mode = %d, fout_name = %s\n", args_num, bg_mode, output_mode, fout_name);
		
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
			strcat(arg_cat, arg);	// concatenate arguments before redirection or background
			if(i != args_num-1) {
				strcat(arg_cat, " ");
			}
		}
		args[args_num] = NULL;
		// printf("cat: %s\n", arg_cat);
		
		
		if(strcmp(args[0], "cd") == 0) {
			// arg = strtok(NULL, "");
			// printf("second: %s\n", arg);
			chdir(args[1]);
		}
		else if(strcmp(args[0], "pwd") == 0) {
			printf("%s\n", buffer);
		}
		else if(strcmp(args[0], "echo") == 0) {
			// arg = strtok(NULL, "");
			printf("%s\n", arg_cat);
		}
		else if(strcmp(args[0], "export") == 0) {
			// printf("PATH: %s\n", getenv("PATH"));
			// arg = strtok(NULL, "");
			// printf("%s\n", arg);
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
			printf("We also support background execution (&) and output redirection (>, >>)\n\n");
			printf("------------------------------------------------\n\n");
		}
		else if(strcmp(args[0], "history") == 0) {
			HIST_ENTRY **hist_list = history_list();
			if(hist_list) {
				printf("Total history in buffer: %d\n", history_length);
				if(args_num == 1) {
					for(i = 0; hist_list[i]; i++) {
						printf("%4d: %s\n", i+history_base, hist_list[i]->line);
					}
				}
				else {
					if(isNumber(args[1])) {
						int hist_offset = atoi(args[1]);
						for(i = max(history_length-hist_offset, 0); i < history_length; i++) {
							printf("%4d: %s\n", i+history_base, hist_list[i]->line);
						}					
					}
					else {
						if(strcmp(args[1], "-clear") == 0) {
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
		else {
			pid_t pid;
			pid = fork();
			if(pid < 0) {
				printf("ERROR: fork\n");
				exit(-1);
			}
			else if (pid == 0) {	// child process
				if(execvp(args[0], args) == -1) {
					printf("ERROR: Command '%s' not found.\n", args[0]);
				}
				exit(0);
			}
			else {		// parent process
				if(bg_mode == 0) {
					wait(NULL);
				}
				// printf("pid = %d, child pid = %d\n", getpid(), pid);
			}
		}
		
		fflush(stdout);
		if(output_mode == 1 || output_mode == 2) {
			dup2(copy, 1);
			fclose(fout);
		}
	}
}

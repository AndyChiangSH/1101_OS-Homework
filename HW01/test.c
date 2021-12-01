#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

int main() {
	using_history();
	read_history("history_file");
	
	add_history("test 111");
	add_history("test 222");
	
	int i, max = 10;
	HIST_ENTRY **hist_list = history_list();
	if(hist_list) {
		for(i = 0; hist_list[i] && i < max; i++) {
			printf("%d: %s\n", i+history_base, hist_list[i]->line);
		}	
	}
	else {
		printf("no history now!\n");
	}
	
	printf("history_length: %d\n", history_length);
	
	write_history("history_file");

	return 0;
}

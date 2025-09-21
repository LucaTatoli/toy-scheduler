#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct command_file* createCommandFile(int line_size) {
	struct command_file *cf = malloc(sizeof(*cf)); 
	cf->command = calloc(5, sizeof(char));
	cf->arg1    = calloc(line_size, sizeof(char));
	cf->arg2    = calloc(line_size, sizeof(char));
	cf->arg3    = calloc(line_size, sizeof(char));
	cf->next_command = NULL;
	return cf;
}

struct command* createCommand(int line_size) {
	struct command *c = malloc(sizeof(*c)); 
	c->command = calloc(5, sizeof(char));
	c->arg1    = calloc(line_size, sizeof(char));
	c->arg2    = calloc(line_size, sizeof(char));
	c->arg3    = calloc(line_size, sizeof(char));
	return c;
}

#define COMMAND_FILE_MAX_SIZE 100
struct command_file2* createCommandFile2() {
	struct command_file2 *cf = malloc(sizeof(*cf));
	cf->pc = 0;
	cf->lines = 0;
	cf->commands = malloc(sizeof(*(cf->commands))*COMMAND_FILE_MAX_SIZE);
	return cf;
}

#define LINE_LENGTH 100
struct command_file* readCommandFile(char* path) {
	FILE *f = fopen(path, "r");
	if(f == NULL) {
		printf("ERRORE: file %s non trovato!\n", path);
		exit(1);
	}
	char ch;

	struct command_file *cf_head = createCommandFile(LINE_LENGTH);

	struct command_file *cf = cf_head;

	char *command_cursor = cf->command;
	int cursor_pos = 0;
	int quotesNum = 0;

	int i = 0;
	while((ch = fgetc(f)) != EOF) {
		if(ch == '"') quotesNum = (quotesNum + 1) % 2;

		if(ch == '\n') {
			command_cursor[i] = 0;
			i = 0;
			cursor_pos = 0;
			struct command_file *next = createCommandFile(LINE_LENGTH);
			cf->next_command = next;
			cf = next;
			command_cursor = cf->command;
		} else if(ch == ' ' && quotesNum == 0) {
			command_cursor[i] = 0;
			cursor_pos++;
			i = 0;
			switch(cursor_pos) {
				case 1:
			 		command_cursor = cf->arg1;
					break;
				case 2:
					command_cursor = cf->arg2;
					break;
				case 3:
					command_cursor = cf->arg3;
					break;
				default:
					printf("ERRORE: too many arguments\n");
					exit(1);
			}		
		}
		else {
			command_cursor[i++] = ch;
		}
	}
	return cf_head;
}

struct command_file2* readCommandFile2(char* path) {
	FILE *f = fopen(path, "r");
	if(f == NULL) {
		printf("ERRORE: file %s non trovato!\n", path);
		exit(1);
	}
	char ch;

	struct command_file2 *cf = createCommandFile2();
	cf->commands[0] = *createCommand(LINE_LENGTH);
	char *command_cursor = cf->commands[0].command;

	int cursor_pos = 0;
	int quotesNum = 0;
	int resizeCount = 1;

	int i = 0;
	while((ch = fgetc(f)) != EOF) {
		if(ch == '"') quotesNum = (quotesNum + 1) % 2;

		if(cursor_pos == 0 && ch == '#') {
			do {
				ch = fgetc(f);
			} while(ch != EOF && ch != '\n');
			continue;
		}
		else if(ch == '\n') {
			i = 0;
			cursor_pos = 0;
			cf->lines++;
			struct command *cmd = createCommand(LINE_LENGTH);
			if(cf->lines - 1 > COMMAND_FILE_MAX_SIZE * resizeCount) {
				resizeCount++;
				resizeCommandFile(cf, resizeCount);
			}
			cf->commands[cf->lines] = *cmd;
			command_cursor = cmd->command;
		} else if(ch == ' ' && quotesNum == 0) {
			cursor_pos++;
			i = 0;
			switch(cursor_pos) {
				case 1:
			 		command_cursor = cf->commands[cf->lines].arg1;
					break;
				case 2:
					command_cursor = cf->commands[cf->lines].arg2;
					break;
				case 3:
					command_cursor = cf->commands[cf->lines].arg3;
					break;
				default:
					printf("ERRORE: too many arguments\n");
					exit(1);
			}		
		}
		else {
			command_cursor[i++] = ch;
		}
	}
	resizeCommandFile(cf, -1);
	return cf;
}

void resizeCommandFile(struct command_file2 *cf, int resizeCount) {
	if(resizeCount <= -1)
		cf->commands = realloc(cf->commands, sizeof(*cf->commands) * cf->lines);
	else
		cf->commands = realloc(cf->commands, sizeof(*cf->commands) * COMMAND_FILE_MAX_SIZE * resizeCount);
}

void print_cf(struct command_file *cf) {
	while(1) {
		if(cf->command[0] == 0)
			break;
		printf("%s %s %s %s", cf->command, cf->arg1, cf->arg2, cf->arg3);
		cf = cf->next_command;
		if(cf == NULL)
		 break;
		printf("\n");
	}
}

void print_cf2(struct command_file2 *cf) {
	int i = 0;
	struct command* cmd = cf->commands;
	while(i < cf->lines) {
		printf("%s %s %s %s", cmd[i].command, cmd[i].arg1, cmd[i].arg2, cmd[i].arg3);
		i++;
		printf("\n");
	}
}

void freeCommandFile(struct command_file *cf) {
	if(cf == NULL)
		return;
	free(cf->command);
	free(cf->arg1);
	free(cf->arg2);
	freeCommandFile(cf->next_command);
	free(cf);
}

void freeCommand(struct command *c) {
	free(c->command);
	free(c->arg1);
	free(c->arg2);
	free(c->arg3);
}

void freeCommandFile2(struct command_file2 *cf) {
	for(int i = 0; i < cf->lines; i++) {
		freeCommand(&(cf->commands[i]));
	}
	free(cf->commands);
	free(cf);
}

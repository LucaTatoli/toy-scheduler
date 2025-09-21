#ifndef COMMAND_H
#define COMMAND_H
struct command_file {
	char *command;
	char *arg1;
	char *arg2;
	char *arg3;
	struct command_file *next_command;
};

struct command {
	char *command;
	char *arg1;
	char *arg2;
	char *arg3;
};

struct command_file2 {
	int pc;
	int lines;
	struct command* commands;
};

struct command_file* createCommandFile(int line_size);
struct command_file2* createCommandFile2();

#define LINE_LENGTH 100
struct command_file* readCommandFile(char* path);
struct command_file2* readCommandFile2(char* path);

void print_cf(struct command_file *cf);
void print_cf2(struct command_file2 *cf);

void freeCommand(struct command *c);
void freeCommandFile(struct command_file *cf);
void freeCommandFile2(struct command_file2 *cf);

void resizeCommandFile(struct command_file2 *cf, int resizeCount);
#endif

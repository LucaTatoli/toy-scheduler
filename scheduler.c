#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"

struct process {
	int id;
	int reg_a;
	int reg_b;
	int reg_c;
	struct command_file2 *command;
};

struct pQueue {
	int head;
	int tail;
	int length;
	struct process* queue[];
};

struct scheduler {
	int maxOperations;
	struct pQueue *readyQueue;
	struct pQueue *screenQueue;
};

struct dummyOS {
	int screenResource;
};

void freeProcess(struct process *p) {
	freeCommandFile2(p->command);
	free(p);
}

void freeQueue(struct pQueue *q) {
	free(q);
}

void freeScheduler(struct scheduler *s) {
	freeQueue(s->readyQueue);
	freeQueue(s->screenQueue);
	free(s);
}

void setReg(struct process *p, int reg, int value) {
	switch(reg) {
		case 0:
			p->reg_a = value;
			break;
		case 1:
			p->reg_b = value;
			break;
		case 2:
			p->reg_c = value;
			break;
		default:
			printf("ERROR: register %d does not exist!\n");
	}
}

int getReg(struct process *p, int reg) {
	switch(reg) {
		case 0:
			return p->reg_a;
		case 1:
			return p->reg_b;
		case 2:
			return p->reg_c;
	}
}

struct process* createProcess(char* path) {
	static int PROCESS_ID = 0;
	struct process *p = malloc(sizeof(*p));
	p->id = PROCESS_ID++;
	p->command = readCommandFile2(path);
	return p;
}

struct pQueue* createQueue(int length) {
	struct pQueue *q = malloc(sizeof(*q)+sizeof(struct process*)*(length+1));
	q->length = length+1;
	q->head = 0;
	q->tail = 0;
	return q;
}

struct scheduler* createScheduler(int maxOperations) {
	struct scheduler *s = malloc(sizeof(*s));
	s->maxOperations = maxOperations;
	s->readyQueue = createQueue(100);
	s->screenQueue = createQueue(100);
	return s;
}

void enqueue(struct pQueue *q, struct process *p) {
	if((q->tail +1) % q->length == q->head) {
		printf("The queue is already full, can't put any other process in the current queue!\n");
		return;
	}
	q->queue[q->tail] = p;
	q->tail = (q->tail + 1) % q->length;
}

struct process* dequeue(struct pQueue *q) {
	if(q->head == q->tail) {
		printf("The queue is empty, no process to dequeue!\n");
		return NULL;
	}
	struct process *p = q->queue[q->head];
	q->head = (q->head + 1) % q->length;
	return p;
}

int askScreenResource(struct dummyOS *os, struct process *p, struct scheduler *s) {
	if(os->screenResource == 0) {
		os->screenResource = 1;
		return 1;
	}
	
	enqueue(s->screenQueue, p);
	return 0;
}

int freeScreenResource(struct dummyOS *os) {
	os->screenResource = 0;
}

void executeWriteCommand(struct process *p, char *string) {
	if(string == NULL) printf("NULL pointer exception!\n");
	printf("OUTPUT: \t");
	int i = 0;
	while(string[i] != 0) {
		char c = string[i++];
		if(c == '"')
			continue;
		if(c == '$') {
			printf("%d", getReg(p, atoi(&(string[i++]))));
		} else if(c == '\\' && string[i] == 'n') {
			printf("\n");
			i++;
		} else {
			printf("%c", c);
		}
	}
}

int parseArg(struct process *p, char *arg) {
	if(arg[0] != '$')
		return atoi(arg);
	
	int reg = atoi(&arg[1]);
	return getReg(p, reg);
}

void executeAddCommand(struct process *p, struct command c) {
	if(c.arg1[0] == 0 || c.arg2[0] == 0 || c.arg3[0] == 0)
		printf("Syntax error on ADD command\n");
	
	int arg1 = parseArg(p, c.arg1);
	int arg2 = parseArg(p, c.arg2);

	setReg(p, atoi(&(c.arg3[1])), arg1+arg2);
}

int executeJumpCommand(struct process *p, struct command c) {
	int jumpLine = parseArg(p, c.arg1) - 1;
	int condition = parseArg(p, c.arg2);
	int conditionValue = parseArg(p, c.arg3);
	if(condition >= conditionValue)
		return 0;
	if(jumpLine < 0 || jumpLine > p->command->lines) {
		printf("ERROR: cannot jump to line %d\n", jumpLine);
		exit(1);
	}
	p->command->pc = jumpLine;
	return 1;
}

int executeLineOfCode(struct process *p, struct scheduler *s, struct dummyOS *os) {
	struct command *cmd = p->command->commands;
	int pc = p->command->pc;
	int command_executed = 0;

	if(strcmp(cmd[pc].command, "WRITE") == 0) {
		if(askScreenResource(os, p, s)) {
			executeWriteCommand(p, cmd[pc].arg1);
			command_executed = 1;
			freeScreenResource(os);
		}
	} else if(strcmp(cmd[pc].command, "SET") == 0) {
		setReg(p, atoi(cmd[pc].arg1), parseArg(p, cmd[pc].arg2));
		command_executed = 1;
	} else if(strcmp(cmd[pc].command, "ADD") == 0) {
		executeAddCommand(p, cmd[pc]);
		command_executed = 1;
	} else if(strcmp(cmd[pc].command, "JUMP") == 0) {	
		command_executed = executeJumpCommand(p, cmd[pc]) == 1 ? 0 : 1;
	}
	else {
		command_executed = 1;
	}

	if(command_executed)
		p->command->pc++;

	if(p->command->pc > p->command->lines)
		return 1;
	return 0;
}

int runProcess(struct process *p, struct scheduler *s, struct  dummyOS *os) {
	int commands_executed = 0;
	printf("RUNNING PROCESS WITH ID: %d\n", p->id);
	while(1) {
		int lastLineFlag = executeLineOfCode(p, s, os);
		commands_executed++;
		if(lastLineFlag) {
			freeProcess(p);
			return 1;
		} else if(commands_executed >= s->maxOperations) {
			return 0;
		}
	}
}

void runDummyOS(struct dummyOS *os, struct scheduler *s) {
	while(1) {
		struct process *p = dequeue(s->readyQueue);
		if(p != NULL) {
			if(!runProcess(p, s, os)) enqueue(s->readyQueue, p);
		} else {
			break;
		}
	}
}

void addProcess(struct scheduler *s, struct process *p) {
	enqueue(s->readyQueue, p);
}

int main() {
	struct dummyOS os;
	struct process *p1 = createProcess("cmd_test.cf");
	struct process *p2 = createProcess("cmd_test2.cf");
	struct process *p3 = createProcess("cmd.cf");
	struct process *p4 = createProcess("cmd2.cf");
	struct process *p5 = createProcess("fibonacci.cf");
	struct scheduler *s = createScheduler(3);
	addProcess(s, p1);
	addProcess(s, p2);
	addProcess(s, p3);
	addProcess(s, p4);
	addProcess(s, p5);
	runDummyOS(&os, s);
	freeScheduler(s);
	return 0;
}

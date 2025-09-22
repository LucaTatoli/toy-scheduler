# Toy Scheduler in C
This is a toy project which implements a scheduler in C, it features a ready queue and screen queue for the processes waiting for the exclusive access to the screen (in this toy project those processes that need to read from the user input). The scheduler executes N lines of command for each process, N is a value that can be set. 

## Process
The process is a structure which holds this data:

 - process id
 - 3 registers
 - a structure that holds the lines of code to be executed
 
## Command file
I made a mini scripting language, assembly like, that is used by the processes. The language features this operations:
 - SET (saves a value into a register)
 - WRITE (outputs to the terminal)
 - ADD (add sto values)
 - JUMP (jumps to a specific line of code based on a condition)
 - READ (reads input from the user {to do})


![The program running](https://imgur.com/OnsbSBT.png)


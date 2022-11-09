#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <time.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported


int global;
int status;
int bool;
int counter1 = 0;
int counter2 = 0;

// Function to take input
int takeInput(char* str)
{
    if (bool == 1)
        counter2++;
    bool = 0;
    char* buf;
    buf = readline("\n>>> ");
    if (strlen(buf) != 0) {
        if (buf[strlen(buf)-1] == '&') {        // Hier soll d
            bool = 1;
            buf[strlen(buf)-1] = ' ';
        }
        add_history(buf);       // gibt uns die Möglichkeit durch den Verlauf mit arrows zu suchen
        strcpy(str, buf);

        return 0;
    } else {
        return 1;
    }
}

// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd);
}

// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
            if (execvp(parsed[0], parsed) < 0) {
                printf("\nCould not execute command..");
            exit(0);
        }
    } else {

        if (bool == 1) {
            printf("pid = %d \n", getpid());
            setpgid(0,0);
        }
        else        // waiting for child to terminate
        wait(&status);
        return;
    }
}

// Help command builtin
void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
         "\nList of Commands supported:"
         "\n>cd"
         "\n>ls"
         "\n>exit"
         "\n>all other general commands available in UNIX shell"
         "\n>improper space handling");

    return;
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed) {
    int NoOfOwnCmds = 4, i, switchOwnArg = 0;
    char *ListOfOwnCmds[NoOfOwnCmds];
    char *username;

    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";

    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }
    clock_t ends = clock();
    switch (switchOwnArg) {
        case 1:
            printf("\nGoodbye\n");
            printf("Time elapsed %f\n", (double) (ends - global) / CLOCKS_PER_SEC);
            exit(0);
        case 2:
            chdir(parsed[1]);
            return 1;
        case 3:
            openHelp();
            return 1;
        case 4:
            username = getenv("USER");
            printf("\nHello %s.\n This is test "
                   "\nUse help to know more..\n",
                   username);
            return 1;
        default:
            break;
    }
    return 0;
}
// function for parsing command words
void parseSpace(char* str, char** parsed)
{
    int i;

   for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");

           if (parsed[i] == NULL)
               break;
           if (strlen(parsed[i]) == 0)
               i--;

    }
}

int processString(char* str, char** parsed)
{
    int piped = 0;

        parseSpace(str, parsed);

    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}

int main()
{
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    int execFlag = 0;
    int PID_Child;
    clock_t start = clock();
    global = start;

    while (1) {


        // print shell line
        printDir();
        // take input
        if (takeInput(inputString))
            continue;
        // process
        execFlag = processString(inputString,
                                 parsedArgs);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command

        // execute
        if (execFlag == 1)
            execArgs(parsedArgs);
        if (bool == 1){
            PID_Child = getpid();
        }
        if (counter2 > counter1) {
            printf("Child with ID done %d", PID_Child);
            counter1++;
        }

        waitpid(-1, NULL, WNOHANG); // clean zombies


    }

    return 0;
}
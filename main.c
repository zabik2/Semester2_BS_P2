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


int TimeCounter = 0;     // erforderlich für den Timecounter
int status;     // wartet auf Child
int ChildHintergrund;           // sorgt für spezielle Ausgabe bei "&"
int ChildID[10];            // Speichert Child pid im Hintergrund


void handleSIGTSTP(int signum){
    printf ("\n ************** Ctrl+C pressed ********");
    clock_t ends = clock();
    printf("\nTime elapsed %f\n", (double) (ends - TimeCounter) / CLOCKS_PER_SEC);
    exit(signum);
}

static void exitListener (int sig){
    pid_t childpid;
    int status;
    for (int i = 0; i < 9; i++){
        childpid = waitpid(ChildID[i], &status, WNOHANG);
        if ((childpid > 0) && (WIFEXITED(status))){
            printf ("\nProzess Nr. %d wurde beendet  \n", ChildID[i]);
        }
    }
}

// Function to take input
int takeInput(char* str)
{
    ChildHintergrund = 0;
    char* buf;
    buf = readline("\n>>> ");
    if (strlen(buf) != 0) {
        if (buf[strlen(buf)-1] == '&') {        // Sorgt für das Lesen der Befehle im Hintergrund, durch das Ersetzen des Symbols
            ChildHintergrund = 1;
            buf[strlen(buf)-1] = ' ';
        }
        add_history(buf);       // gibt uns die Möglichkeit durch den Verlauf mit arrows zu suchen
        strcpy(str, buf);

        return 0;
    } else {
        return 1;
    }
}

// Gibt uns die aktuelle Dir
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd);
}

// Function mit execute der Kommando
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork();
    int increment = 0;

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
            if (execvp(parsed[0], parsed) < 0) {
                printf("\nCould not execute command..");
            exit(0);
        }
    } else {

        if (ChildHintergrund == 1) { // child im Hintergrund
            printf("pid = %d \n", pid);
            ChildID[increment] = pid;
            increment++;
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
         "\n>hello"
         "\n>exit"
         "\n>all other general commands available in UNIX shell");

    return;
}

// Function mit builtin Kommandos
int ownCmdHandler(char** parsed) {
    int NoOfOwnCmds = 4, i, switchOwnArg = 0;
    char *ListOfOwnCmds[NoOfOwnCmds];
    char *username;
    char ExitTrue;

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
            printf("Do you really want to exit? Y/N \n");
            scanf("%c", &ExitTrue);
            if (ExitTrue == 'Y') {
                printf("\nGoodbye\n");
                printf("Time elapsed %f\n", (double) (ends - TimeCounter) / CLOCKS_PER_SEC);
                exit(0);
            }
            else
                return 1;
        case 2:
            chdir(parsed[1]);
            if (parsed[1] == NULL) // für "cd" (ohne Parameter)
                chdir(getenv("HOME"));
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
// function erlaubt mehrere Wörter in Kommandos
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
        parseSpace(str, parsed);

    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1;
}

int main()
{
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    int execFlag = 0;
    clock_t start = clock();
    TimeCounter = start;

    while (1) {

        signal (SIGINT, handleSIGTSTP);
        signal (SIGCHLD, exitListener);
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

        waitpid(-1, NULL, WNOHANG); // clean zombies


    }

    return 0;
}
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAX_BUFFER 1024    // max line buffer
#define MAX_ARGS 64        // max # args
#define SEPARATORS " \t\n" // token sparators
#define README "readme"    // help file name

struct shellstatus_st
{
    int foreground;  // foreground execution flag
    char *infile;    // input redirection flag & file
    char *outfile;   // output redirection flag & file
    char *outmode;   // output redirection mode
    char *shellpath; // full pathname of shell
};
typedef struct shellstatus_st shellstatus;

extern char **environ;

void check4redirection(char **, shellstatus *); // check command line for i/o redirection
void errmsg(char *, char *);                    // error message printout
void execute(char **, shellstatus);             // execute command from arg array
char *getcwdstr(char *, int);                   // get current work directory string
FILE *redirected_op(shellstatus);               // return required o/p stream
char *stripath(char *);                         // strip path from filename
void syserrmsg(char *, char *);                 // system error message printout

/*******************************************************************/

int main(int argc, char **argv)
{
    FILE *ostream = stdout;   // (redirected) o/p stream
    FILE *instream = stdin;   // batch/keyboard input
    char linebuf[MAX_BUFFER]; // line buffer
    char cwdbuf[MAX_BUFFER];  // cwd buffer
    char *args[MAX_ARGS];     // pointers to arg strings
    char **arg;               // working pointer thru args
    char *prompt = "==>";     // shell prompt
    char *readmepath;         // readme pathname
    shellstatus status;       // status structure

    // parse command line for batch input
    switch (argc)
    {
    case 1:
    {
        // keyboard input
        // TODO
        break;
    }

    case 2:
    {
        // possible batch/script
        // TODO
        instream = fopen(argv[1],"r");
        if (instream == NULL){
            errmsg("open file error","check your command");
            exit(1);
        }
        break;
    }
    default: // too many arguments
        fprintf(stderr, "%s command line error; max args exceeded\n"
                        "usage: %s [<scriptfile>]",
                stripath(argv[0]), stripath(argv[0]));
        exit(1);
    }

    //todo preparation
    getcwdstr(cwdbuf, MAX_BUFFER); 
    char *new_pwd = (char*) malloc(MAX_BUFFER); //? `cd `
    //todo change prompt
    prompt = (char*) malloc(MAX_BUFFER);
    strcpy(prompt,cwdbuf);
    strcat(prompt,"==>");
    // get starting cwd to add to readme pathname
    // TODO
    readmepath = (char*) malloc(MAX_BUFFER);
    strcpy(readmepath,cwdbuf);
    strcat(readmepath,"/readme.txt"); 
    //? why bother?

    // get starting cwd to add to shell pathname
    // TODO
    status.shellpath = cwdbuf; //! notice: do not change shellpath or cwdbuf

    // set SHELL= environment variable, malloc and store in environment
    // TODO
    char *new_env = (char*) malloc(MAX_BUFFER);
    strcpy(new_env,"SHELL=");
    strcat(new_env, cwdbuf);
    putenv(new_env);


    // prevent ctrl-C and zombie children
    signal(SIGINT, SIG_IGN);  // prevent ^C interrupt
    signal(SIGCHLD, SIG_IGN); // prevent Zombie children

    // keep reading input until "quit" command or eof of redirected input
    while (!feof(instream))
    {
        // (re)initialise status structure
        status.foreground = TRUE;

        // set up prompt
        // TODO
        fputs(prompt, stdout); // write prompt
        fflush(stdout);
        // get command line from input
        if (fgets(linebuf, MAX_BUFFER, instream))
        {
            // read a line
            // tokenize the input into args array
            arg = args;
            *arg++ = strtok(linebuf, SEPARATORS); // tokenize input
            while ((*arg++ = strtok(NULL, SEPARATORS)))
                ;

            // last entry will be NULL
            if (args[0])
            {
                // check for i/o redirection
                check4redirection(args, &status);

                // check for internal/external commands
                // "cd" command
                if (!strcmp(args[0], "cd")) //done
                {
                    // TODO 
                    // getcwdstr(cwdbuf,MAX_BUFFER); 
                    if (args[1]){
                        chdir(args[1]);
                        //TODO change PWD
                        strcpy(new_pwd, "PWD=");
                        strcat(new_pwd, cwdbuf);
                        putenv(new_pwd);
                    }
                    else printf(cwdbuf);
                }
                // "clr" command
                else if (!strcmp(args[0], "clr"))
                {
                    // TODO
                    args[0] = "clear";
                    // args[1] = NULL; //not neccessary
                    execute(args,status); //! the status may need to be changed
                }
                // "dir" command
                else if (!strcmp(args[0], "dir"))
                {
                    // TODO
                    args[0] = "ls";
                    char *tmp = args[1];
                    args[1] = "-al";
                    args[2] = tmp;
                    if (!tmp){
                        args[2] = ".";
                    }
                    args[3] = NULL;
                    execute(args,status); //! the status may need to be changed

                }
                // "echo" command
                else if (!strcmp(args[0], "echo"))
                {
                    // TODO
                    execute(args,status); //! the status may need to be changed
                }
                // "environ" command
                else if (!strcmp(args[0], "environ"))
                {
                    // TODO
                    char **envstr = environ;
                    while (*envstr)
                    { 
                        // print out environment
                        printf("%s\n", *envstr);
                        envstr++;
                    } 
                }
                // "help" command
                else if (!strcmp(args[0], "help"))
                {
                    args[0] = "more";
                    args[1] = readmepath;
                    args[2] = NULL;

                    execute(args,status); //! the status may need to be changed
                }
                // "pause" command - note use of getpass - this is a made to measure way of turning off
                //  keyboard echo and returning when enter/return is pressed
                else if (!strcmp(args[0], "pause")) //done
                {
                    // TODO
                    printf("Press Enter to continue\n");
                    while (1){
                        char *password = getpass("");
                        if (strlen(password) == 0){
                            break;
                        }
                    }
                }
                // "quit" command
                else if (!strcmp(args[0], "quit")) 
                {
                    
                    break;
                }
                
                // else pass command on to OS shell
                // TODO
                else{
                    execute(args,status); //* external command(linux)
                }
            }
        }
    }
    //todo free malloc
    free(readmepath);
    free(new_env);
    free(new_pwd);
    return 0;
}

/***********************************************************************

void check4redirection(char ** args, shellstatus *sstatus);

check command line args for i/o redirection & background symbols
set flags etc in *sstatus as appropriate

***********************************************************************/

void check4redirection(char **args, shellstatus *sstatus)
{
    sstatus->infile = NULL; // set defaults
    sstatus->outfile = NULL;
    sstatus->outmode = NULL;

    while (*args)
    {
        // input redirection
        if (!strcmp(*args, "<"))
        {
            // TODO
            *args = NULL;
            sstatus->infile = *(++args);
            // still need to open file
        }
        // output direction
        else if (!strcmp(*args, ">") || !strcmp(*args, ">>"))
        {
            // TODO
            if (!strcmp(*args,">")) sstatus->outmode = "w";
            else sstatus->outmode = "a";
            *args = NULL;
            sstatus->outfile = *(++args);
            
        }
        else if (!strcmp(*args, "&"))
        {
            // TODO
            *args = NULL;
            sstatus->foreground = FALSE;
        }
        args++; // will not change main:args
    }
}

/***********************************************************************

  void execute(char ** args, shellstatus sstatus);

  fork and exec the program and command line arguments in args
  if foreground flag is TRUE, wait until pgm completes before
    returning

***********************************************************************/

void execute(char **args, shellstatus sstatus)
{
    int status;
    pid_t child_pid;
    char tempbuf[MAX_BUFFER];
    
    switch (child_pid = fork())
    {
    case -1:
        syserrmsg("fork", NULL);
        break;
    case 0:
        // execution in child process
        // reset ctrl-C and child process signal trap
        signal(SIGINT, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        // i/o redirection */
        // TODO
        if (sstatus.infile){
            //? cd,echo do not need input, probably not neccessary
            freopen(sstatus.infile, "r", stdin);
        }
        if (sstatus.outfile) redirected_op(sstatus);

        // set PARENT = environment variable, malloc and put in nenvironment
        // TODO
        char *parent = (char*) malloc(MAX_BUFFER);
        strcpy(parent, "PARENT=");
        strcat(parent, sstatus.shellpath);

        // final exec of program
        execvp(args[0], args);
        syserrmsg("exec failed -", args[0]);
        exit(127);
    default:
        wait(NULL);
        // break;
    }

    // continued execution in parent process
    // TODO
    // waitpid(child_pid, &status, sstatus.foreground); // linux: WNOHANG := 1
    //? need to check status?
}

/***********************************************************************

 char * getcwdstr(char * buffer, int size);

return start of buffer containing current working directory pathname

***********************************************************************/

char *getcwdstr(char *buffer, int size)
{
    // TODO
    if (getcwd(buffer, size) == NULL) {
        errmsg("directory not found",NULL);
        exit(1);
    }
    return buffer;
}

/***********************************************************************

FILE * redirected_op(shellstatus ststus)

  return o/p stream (redirected if necessary)

***********************************************************************/

FILE *redirected_op(shellstatus status)
{
    FILE *ostream = stdout;

    // TODO
    freopen(status.outfile, status.outmode, ostream);
    return ostream;
}

/*******************************************************************

  char * stripath(char * pathname);

  strip path from file name

  pathname - file name, with or without leading path

  returns pointer to file name part of pathname
            if NULL or pathname is a directory ending in a '/'
                returns NULL
*******************************************************************/

char *stripath(char *pathname)
{
    //* remove directory 
    char *filename = pathname;

    if (filename && *filename)
    {                                      // non-zero length string
        filename = strrchr(filename, '/'); // look for last '/'
        // ! notice strchr and strrchr
        if (filename)                      // found it
            if (*(++filename))             //  AND file name exists
                return filename;
            else
                return NULL;
        else
            return pathname; // no '/' but non-zero length string
    }                        // original must be file name only
    return NULL;
}

/********************************************************************

void errmsg(char * msg1, char * msg2);

print an error message (or two) on stderr

if msg2 is NULL only msg1 is printed
if msg1 is NULL only "ERROR: " is printed
*******************************************************************/

void errmsg(char *msg1, char *msg2)
{
    fprintf(stderr, "ERROR: ");
    if (msg1)
        fprintf(stderr, "%s; ", msg1);
    if (msg2)
        fprintf(stderr, "%s; ", msg2);
    return;
    fprintf(stderr, "\n");
}

/********************************************************************

  void syserrmsg(char * msg1, char * msg2);

  print an error message (or two) on stderr followed by system error
  message.

  if msg2 is NULL only msg1 and system message is printed
  if msg1 is NULL only the system message is printed
 *******************************************************************/

void syserrmsg(char *msg1, char *msg2)
{
    errmsg(msg1, msg2);
    perror(NULL);
    return;
}

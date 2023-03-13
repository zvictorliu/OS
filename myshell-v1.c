/*

  the version mainly relys on "execvp()"

 *********************************************************************
  version: 1.0
 *******************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024    // max line buffer
#define MAX_ARGS 64        // max # args
#define SEPARATORS " \t\n" // token sparators

extern char **environ;
/*******************************************************************/

int main(int argc, char **argv)
{
    char linebuf[MAX_BUFFER]; // line buffer
    char cmndbuf[MAX_BUFFER]; // command buffer
    char *args[MAX_ARGS];     // pointers to arg strings
    char **arg;               // working pointer thru args
    char *prompt = "==>";     // shell prompt

    char *cmnd[MAX_ARGS]; //cmnd strings
    // keep reading input until "quit" command or eof of redirected input
    while ( !feof(stdin))
    {
        // get command line from input
        fputs(prompt, stdout); // write prompt
        fflush(stdout);
        if (fgets(linebuf, MAX_BUFFER, stdin))
        { 
            // read a line
            // tokenize the input into args array
            arg = args;
            *arg++ = strtok(linebuf, SEPARATORS); // tokenize input
            // 含义是 *(args++)，当然第一次不加
            // 忽略换行符，可以理解为按空格分割
            // 返回第一个
            while ((*arg++ = strtok(NULL, SEPARATORS)))
                ;
            // 之后调用strok只需传入NULL，第一次调用会“记住”，并且会缩短
            // last entry will be NULL 以空指针结尾

            if (args[0])
            { 
                // if there's anything there
                cmndbuf[0] = 0; // set zero-length command string

                // check for internal/external command
                if (!strcmp(args[0], "clr"))
                { 
                    // "clr" command
                    strcpy(cmndbuf, "clear");
                }
                else if (!strcmp(args[0], "help")){
                    strcpy(cmndbuf,"echo there is nothing I can help");
                }
                else if (!strcmp(args[0], "more")){
                    strcpy(cmndbuf,"cat readme.txt");
                } 
                else if (!strcmp(args[0], "dir"))
                { 
                    // "dir" command
                    strcpy(cmndbuf, "ls -al ");
                    if (!args[1])
                        args[1] = "."; // if no arg set current directory
                    strcat(cmndbuf, args[1]);

                }
                else if (!strcmp(args[0], "environ"))
                { 
                    // "environ" command
                    char **envstr = environ;
                    while (*envstr)
                    { 
                        // print out environment
                        printf("%s\n", *envstr);
                        envstr++;
                    } 
                    // (no entry in cmndbuf)
                }
                else if (!strcmp(args[0], "quit"))
                { 
                    // "quit" command

                    break; // just end
                }
                else if (!strcmp(args[0],"pause")){
                    printf("Press Enter to continue\n");
                    while (1){
                        char *password = getpass("");
                        if (strlen(password) == 0){
                            break;
                        }
                    }
                    
                }
                else if (!strcmp(args[0],"cd")){
                    if (args[1]){
                        chdir(args[1]);
                    }
                    else{
                        char buf[100];
                        if (getcwd(buf, sizeof(buf)) == NULL) {
                            perror("error: directory not found");
                        }
                        else printf("The current working directory is: %s\n",buf);
                    }
                }
                else
                { 
                    // pass command on to OS shell
                    int i = 1;
                    strcpy(cmndbuf, args[0]);
                    while (args[i])
                    {
                        strcat(cmndbuf, " ");
                        strcat(cmndbuf, args[i++]);
                    }
                }
            
            

                // pass any command onto OS
                if (cmndbuf[0]) {
                    // system(cmndbuf);
                    // ! cmnd是一条完整指令(linux)字符串，不能直接用execvp
                    int i = 0;
                    char *token = strtok(cmndbuf," \n");
                    while (token !=NULL){
                        cmnd[i] = token;
                        i++;
                        token = strtok(NULL," \n");
                    }
                    cmnd[i] = NULL; //要求以空指针结尾

                    //使用execvp
                    pid_t pid = fork();

                    if (pid < 0){
                        // error
                        fprintf(stderr,"fork failed\n");
                        return 1;
                    }
                    else if (pid == 0){
                        // child process
                        execvp(cmnd[0],cmnd);
                        printf("Command not found\n");
                        exit(1);
                    }
                    else {
                        // parent process waits
                        wait(NULL);
                        // printf("Child process finished\n");
                    }
                }
                    
                    


            }
        }
    }
    return 0;
}

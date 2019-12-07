#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

///Global Variables
char cdir[1028];        //current work directory for logging


///Functions prototypes

/**Helping functions*/
void strPrint(char *buf);
void strRead(char* buf);
void cpySubstr(char *src, char *dest, int a, int b);

/**Shell environment setup*/
void getUsername(char *buf);
void getCurrentDir(char* buf);
void setLogsPath();
void printhead();

/**Shell functions*/
void launchShell();
int isBackgroundOp(char* buf);
void splitInput(char *args[100], char *buf);
void cmdHandle(char **args, int runbackground);
void childSignalHandler(int sig);

int main()
{
    setLogsPath();
    signal(SIGCHLD, childSignalHandler);
    while (1)
    {
        launchShell();
    }
    return 0;
}

/**Helping functions*/
void strPrint(char *buf)
{
    int i=0;
    while(buf[i])
    {
        printf("%c", buf[i]);
        i++;
    }
}

void strRead(char *buf)
{
    fgets(buf, 1028, stdin);
    int i=0;
    while(buf[i] != '\0') i++;
    while(buf[i-1] == '\n' || buf[i-1] == ' ')
    {
        buf[i-1] = '\0';
        i--;
    }
}

void cpySubstr(char *src, char *dest, int a, int b)
{
    char *p = NULL;
    int i=0;
    while(a <= b)
    {
        p[i] = src[a];
        a++;
        i++;
    }
    p[i] = '\0';
    dest = p;
}

/**Shell environment setup*/
void getUsername(char *buf)
{
    getlogin_r(buf, sizeof(buf));
}

void getCurrentDir(char *buf)
{
    getcwd(buf, 1024);
}

void setLogsPath()
{
    getCurrentDir(cdir);
    strcat(cdir, "/logs.txt");
}

void printhead()
{
    char uname[20];
    char hname[50];
    getUsername(uname);
    gethostname(hname, sizeof(hname));
    strPrint(uname);
    printf("@");
    strPrint(hname);
    printf(":~$\t");
}

/**Shell functions*/
void launchShell()
{
    printhead();
    char buf[200];
    strRead(buf);
    int runBackground = isBackgroundOp(buf);
    char *args[100];
    splitInput(args, buf);
    cmdHandle(args, runBackground);
}

int isBackgroundOp(char* buf)
{
    int strend = strlen(buf) - 1;
    if (buf[strend] == '&')
    {
        buf[strlen(buf) - 1] = '\0';
        while(buf[strend] == ' ' && strend>=0)
        {
            buf[strend] = '\0';
            strend--;
        }
        return 1;
    }
    return 0;
}

void splitInput(char *args[], char *buf)
{
    //IF command is cd THEN there is one parameter only
    if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ')
    {
        args[0] = "cd";
        //cpySubstr(buf, args[1], 3, strlen(buf)-1);
        buf += 3;
        args[1] = buf;
        args[2] = NULL;
    }
    else {
        char *temp = strtok(buf, " ");
        int i = 0;
        while(temp != NULL)
        {
            args[i] = temp;
            temp = strtok(NULL, " ");
            i++;
        }
        args[i] = NULL;
    }
}

void cmdHandle(char **args, int runbackground)
{
    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0)
    {
        chdir(args[1]);
    }
    else {
        pid_t pid = fork();
        if (pid == 0)
        {
            execvp(args[0], args);

        }
        else {
            if (!runbackground)
            {
                int state;
                waitpid(pid, &state, WUNTRACED);
            }
        }
    }
}

void childSignalHandler(int sig)
{
    // get current date and time
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    //write in logs file "logs.txt"
    FILE *logs;
    logs = fopen(cdir, "a");
    fprintf(logs, "%d-%d-%d %02d:%02d:%02d : Child process was terminated\n", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fclose(logs);
}

/*
Name: G.Aishwarya
Date: 05-04-2026
project:  MiniShell

Description: This is the function file for the MINIshell project, 
it includes all the functions regarding to the project 
*/

#include "header.h"

char prompt[50] = "MiniShell:$ ";   // shell prompt
char input_string[100];             // user input
int status;                        // status of last process
extern char buf[800];                     // buffer
Slist *head = NULL;                // job list head
int job_id = 1;             // auto increment job id
pid_t fg_pid = 0;                  // current foreground process

void insert_at_last(pid_t pid, char *cmd)
{

    Slist *new = malloc(sizeof(Slist)); // create new node
    if (new==NULL)
    {
        perror("error");
        return;
    }

    new->job_id = job_id++;         //  assign job id
    new->pid = pid;             // assign pid
    strcpy(new->cmd, cmd);      // assign command
    new->flag = 1;                 // mark as stopped
    new->link = NULL;           // insert at end of list

    if (head == NULL)           // if list is empty
    {
        head = new;
    }
    else
    {
        Slist *temp = head;         //  traverse to end of list
        while (temp->link)
        {
            temp = temp->link;          // move to next
        }
        temp->link = new;           // insert new node
    }
}

void signal_handler(int sig_num)        // handle signals   
{
    if (sig_num == SIGINT) // Ctrl + C
    {
        if (fg_pid != 0)
        {
            kill(fg_pid, SIGINT); // send signal to process
        }
        else
        {
            printf( "\n%s" , prompt);
            fflush(stdout);
        }
    }
    else if (sig_num == SIGTSTP) // Ctrl + Z
    {
        if (fg_pid != 0)
        {
            kill(fg_pid, SIGTSTP);
        }
        else
        {
            printf( "\n%s" , prompt);
            fflush(stdout);
        }
    }
    else if (sig_num == SIGCHLD) // child finished
    {
        int stat;
        pid_t pid;

        while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)     // check for finished child processes
        {
            job_id--; // decrement job id

            Slist *temp = head, *prev = NULL;       // find finished process in job list

            while (temp)            // traverse job list
            {
                if (temp->pid == pid)
                {
                    if (prev)
                    {
                        prev->link = temp->link;        // remove from list
                    }
                    else
                    {
                        head = temp->link;          // remove head if first job
                    }
                    free(temp);
                    break;
                }
                prev = temp;            // move to next
                temp = temp->link;       // move to next
            }
        }
    }
}

void extract_external_commands(char **external_commands)        // load external commands from file
{
    int fd = open("external_cmd.txt", O_RDONLY);        // open file with external commands
    if (fd < 0)
    {
        perror("open");
        return;
    }

    char ch, temp[50];
    int i = 0, index = 0;

    while (read(fd, &ch, 1) > 0)    // read character by character
    {
        if (ch != '\n')
        {
            temp[i++] = ch;         // build command until newline
        }
        else
        {
            temp[i] = '\0';
            external_commands[index] = malloc(strlen(temp) + 1);        // allocate memory for command
            strcpy(external_commands[index++], temp);               // copy command to list
            i = 0;
        }
    }
    if (i > 0)      // last command without newline
    {
        temp[i] = '\0';
        external_commands[index] = malloc(strlen(temp) + 1);            // allocate memory for command
        strcpy(external_commands[index++], temp);                   // copy command to list
    }
    external_commands[index] = NULL;    
    close(fd);
}

char *get_command(char *input_string)   // extract first word from input
{
    static char cmd[20];        // buffer for command word
    int i = 0;

    while (input_string[i] != ' ' && input_string[i] != '\0')   // copy until space or end
    {
        cmd[i] = input_string[i];           // build command word
        i++;
    }
    cmd[i] = '\0';
    return cmd;
}

int check_command_type(char *command)   // check if command is builtin or external
{
    if (strncmp(command, "echo", 4) == 0)
    {
        return BUILTIN;   //    echo is builtin
    }
    char *internal_cmds[] = {"cd","pwd","exit","clear", NULL};

    for (int i = 0;internal_cmds[i]; i++)       // check against builtin list
    {
        if (strcmp(command, internal_cmds[i]) == 0)     // found in builtin list
        {
            return BUILTIN;
        }
    }
    return EXTERNAL;
}

void execute_internal_commands(char *input_string)      // execute builtin commands
{
    if(strcmp(input_string,"exit")==0)  // exit command
    {
        exit(0);
    }

    else if (strcmp(input_string, "pwd") == 0)      // print working directory
    {
        char buffer[100];
        getcwd(buffer, sizeof(buffer));
        printf(ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, buffer);
    }

    else if (strncmp(input_string, "cd ", 3) == 0)      // change directory
    {
        if (chdir(input_string + 3) != 0)   //it will change the directory to the one specified after "cd "
            perror("cd");
    }

    else if (strncmp(input_string, "echo", 4) == 0) // echo command with special variables
    {
        char *echo_info = input_string + 4;

        while (*echo_info == ' ')       // skip spaces
        {
            echo_info++;  
        }
        if (strcmp(echo_info, "$$") == 0)       // print shell pid
        {
            printf("%d\n", getpid());
        }
        else if (strcmp(echo_info, "$?") == 0)      //  print last command exit status
        {
            printf("%d\n", WEXITSTATUS(status));        //to terminate the child process and get the exit status of the last command
        }
        else if (strcmp(echo_info, "$SHELL") == 0)      // print shell name
        {
            printf("%s\n", buf);
        }
        else            // normal echo
        {
            printf("%s\n", echo_info);
        }
    }
    else if(strcmp(input_string,"clear")==0)        // clear screen
    {
        system("clear");   
    }
}

void print_list(Slist *head)    // print job list
{
    if (head == NULL)
    {
        printf(ANSI_COLOR_RED "No jobs\n" ANSI_COLOR_RESET);
        return;
    }

    Slist *temp = head;     // traverse job list

    while (temp)
    {
        printf(ANSI_COLOR_GREEN "[%d] ^Z Stopped %s\n" ANSI_COLOR_RESET, temp->job_id, temp->cmd);
        temp = temp->link;
    }
}

void execute_external_commands(char *input)     // execute external commands, handle pipes
{
    char *commands[10];
    int num_cmds = 0;

    char temp[100];
    strcpy(temp, input);

    char *token = strtok(temp, "|");    // split by pipe
    while (token)
    {
        while (*token == ' ') token++;  // skip spaces
        commands[num_cmds++] = token;
        token = strtok(NULL, "|");      // next command
    }
    if (num_cmds == 1)      // no pipe, single command
    {
        char *argv[20];
        int i = 0;

        char temp2[100];
        strcpy(temp2, commands[0]);     // copy command

        char *tok = strtok(temp2, " ");     // split by space for arguments
        while (tok)
        {
            argv[i++] = tok;
            tok = strtok(NULL, " ");        // next argument
        }
        argv[i] = NULL;

        execvp(argv[0], argv);      // execute command
        printf("ERROR : WRONG COMMAND USAGE\n");        // exec failed
        exit(1);
    }

    int fd[2];
    int prev_fd = 0;

    for (int i = 0; i < num_cmds; i++)      // handle multiple commands with pipes
    {
        pipe(fd);       // create pipe for communication

        pid_t pid = fork();         //  create child for each command

        if (pid == 0)       // child process
        {
            if (i > 0)
            {
                dup2(prev_fd, 0);       // input from prev
                close(prev_fd);
            }

            if (i < num_cmds - 1)
            {
                dup2(fd[1], 1);         // output to next
            }

            close(fd[0]);

            char *argv[20];
            int j = 0;

            char temp2[100];    
            strcpy(temp2, commands[i]);     //  copy command for parsing

            char *tok = strtok(temp2, " ");
            while (tok)
            {
                argv[j++] = tok;
                tok = strtok(NULL, " ");
            }
            argv[j] = NULL;

            execvp(argv[0], argv);
            printf( "ERROR : WRONG COMMAND USAGE\n");
            exit(1);
        }
        else
        {
            close(fd[1]);
            if (i > 0)      // close previous read end
            {
                close(prev_fd);
            }
            prev_fd = fd[0];        // next read end
        }
    }

    for (int i = 0; i < num_cmds; i++)  // wait for all children to finish
    {
        wait(NULL);
    }
}
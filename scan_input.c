/*
Name: G.Aishwarya
Date: 05-04-2026
Project: MiniShell

Description: This is the source file for the scan_input function in the MiniShell project.
The scan_input function is responsible for continuously taking user input, parsing it, and executing the appropriate commands, 
including handling built-in commands, external commands, and job control.
*/

#include "header.h"

extern char buf[800];      // buffer for path
extern Slist *head;        // job list
extern pid_t fg_pid;       // foreground process id
extern int status;         // last command status

char *word;               // command
int var;                  // command type

void scan_input(char *prompt, char *input_string)       //scanning the inputs
{
    if(getcwd(buf,sizeof(buf)) == NULL)       
    {
        printf("Error\n");
    }

    printf(ANSI_COLOR_BLUE "\n====================== WELCOME TO MINI SHELL AISHWARYA==============================\n" ANSI_COLOR_RESET);

    while (1)
    {
        printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, prompt);        // print shell prompt
        fflush(stdout);

        input_string[0] = '\0'; // clear input
        
        scanf("%[^\n]", input_string);     
        getchar();

        if(strncmp(input_string, "PS1=", 4) == 0)   // changing promt
        {
            if (strchr(input_string, ' '))      // check for spaces in prompt
            {
                printf(ANSI_COLOR_RED "No spaces allowed in PS1\n" ANSI_COLOR_RESET);
            }
            else            // update prompt
            {
                strcpy(prompt, input_string + 4); 
            }
            continue;
        }
        if (strlen(input_string) == 0)
        {
            continue;
        }
        word = get_command(input_string);        // extract command
        var = check_command_type(word);          // check builtin or external

        
        if(strcmp(input_string, "jobs") == 0)       // jobs command
        {
            printf("Listing jobs...\n");
            print_list(head);       // print job list
        }

        else if(strcmp(input_string, "fg") == 0)        // fg command
        {
            if(head == NULL)
            {
                printf( "No jobs\n");    // no jobs to foreground
                continue;
            }

            Slist *temp = head;         // first job in list

            printf( "Running the cmd : %s\n", temp->cmd);       // print job being foregrounded

            kill(temp->pid, SIGCONT);   // resume process

            fg_pid = temp->pid;         // track foreground

            waitpid(temp->pid, &status, WUNTRACED);         // wait for it to finish or stop

            head = temp->link;          // remove from job list
            free(temp);

            printf("Job completed\n");

            fg_pid = 0;         // reset
        }

        else if(strcmp(input_string, "bg") == 0)    // bg command
        {
            if(head == NULL)
            {
                printf(ANSI_COLOR_RED "No jobs\n" ANSI_COLOR_RESET);
                continue;
            }

            Slist *temp = head;         // first job in list

            printf("[%d] %s\n", temp->job_id, temp->cmd);       // print job being backgrounded

            kill(temp->pid, SIGCONT);           // continue in background

            printf(ANSI_COLOR_GREEN "Job running in background\n" ANSI_COLOR_RESET);
        }
        else if(var == BUILTIN)         // builtin command
        {
            execute_internal_commands(input_string); // execute builtin
        }
        else if(var == EXTERNAL)        // external command
        {
            pid_t pid = fork();         // create child process

            if(pid > 0)         // parent process
            {
                fg_pid = pid;       // track foreground process

                waitpid(pid, &status, WUNTRACED);           // wait for it to finish or stop    
                if(WIFSTOPPED(status))          // if stopped, add to job list
                {
                    insert_at_last(pid, input_string); // add to jobs

                    Slist *temp = head;
                    while(temp->link)       // traverse to last job
                    {
                        temp = temp->link;
                    }
                    printf("[%d]+ Stopped %s\n",temp->job_id, input_string);        // print job info
                }
                fg_pid = 0; // reset
            }
            else if(pid == 0)
            {
                signal(SIGINT, SIG_DFL);  // default signal handling
                signal(SIGTSTP, SIG_DFL); // default signal handling
                execute_external_commands(input_string); // run command
                exit(0);
            }
        }
        else        // no command found
        {
            printf(ANSI_COLOR_RED "Error: Wrong Command Usage\n" ANSI_COLOR_RESET);
        }
    }
}
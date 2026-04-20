/*
Name: G.Aishwarya
Date: 05-04-2026
Project: MiniShell

Description: This is the main file for the MiniShell project. 
It implements a simple shell that can execute internal and external commands, handle signals, and manage background jobs.
*/

#include "header.h"

char buf[800];                     // buffer
char *external_commands[800];      // list of external commands

int main()
{
    getcwd(buf,sizeof(buf));    // get current working directory
    system("clear");  // clear screen at start

    // handle signals
    signal(SIGINT, signal_handler);     // Ctrl + C
    signal(SIGTSTP, signal_handler);    // Ctrl + Z
    signal(SIGCHLD, signal_handler); //  child finished

    extract_external_commands(external_commands); // load commands

    scan_input(prompt, input_string); // starting shell

    return 0;
}
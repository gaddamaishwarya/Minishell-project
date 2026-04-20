/*
Name: G.Aishwarya
Date: 05-04-2026
Project: MiniShell

Description: This is the header file for the MiniShell project. 
It contains all the necessary includes, global variable declarations, and function prototypes used across the project.
*/

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// ===== COMMAND TYPES =====
#define BUILTIN     1
#define EXTERNAL    2
#define NOCOMMAND   3

// Colors
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
// #define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// ===== JOB LINKED LIST =====
typedef struct node
{
    int job_id;
    pid_t pid;
    char cmd[100];
    int flag;
    struct node *link;
} Slist;


// ===== GLOBAL VARIABLES =====
extern char prompt[50];
extern char input_string[100];

// ===== FUNCTIONS =====
void scan_input(char *prompt, char *input_string);
char *get_command(char *input_string);

int check_command_type(char *command);
void execute_internal_commands(char *input_string);
void signal_handler(int sig_num);

void extract_external_commands(char **external_commands);
void execute_external_commands(char *input_string);

void print_list(Slist *head);
void insert_at_last(pid_t pid, char *cmd);

#endif
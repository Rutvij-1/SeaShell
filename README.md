# Basic Linux Shell written in C

## Introduction

Linux Shell supporting some basic functionalities written in C with the help of system header files.

## How to use

- Clone the directory
- run `make` inside the directory.
- `./shell` to run the shell.
- `quit` to exit out of the shell.

## Commands Implemented

1. **echo**- Display the arguments extered by user. Implemented by simply printing the arguments in a space seperated way.
1. **pwd**- Display the current working directory of the process calling it. Implemented using the `getcwd()` function.
1. **cd**- Change directory by passing relative or absolute path. Implemented using the `chdir()` function. Also includes functionality of previous working directory, and can be accessed by `cd -`. Implemented by storing the previous working directory path and updating it with the current working directory every time this command is called.
1. **ls**- Displays the contents of folder(if given), otherwise displays contents of current working directory. And accepts flags `-l` and `-a`. Implemented with functions in *dirent.h*, *time.h* and functions like `stat()`, `getpwuid()` and `getgrgid()`.
1. **execvp_bg**- Executes commands in background. It can be done for a command say **cmd** by `cmd &`. Note that this is not applicable on basic commands like *echo, pwd, cd, ls*. Implemented by maintaining a queue of background processes, and adding the given to process to it.
1. **execvp_fg**- Excecutes commands in foreground. Implemented using functions like `tcsetpgrp()` and `setpgid()`. Basically the command is run by the child process, where the parent process waits for it's child to terminate.
1. **history**- Displays the history of inputs passed to the shell, with an optional argument of the number of inputs to be displayed. Can be called by `history <num>`. Implemented by storing the command in a *.txt* file, and storing a buffer of limited size(20 for this shell) in the program memory.
1. **nightswatch**- Periodically displays one of the following two information.
    1. Number of time each CPU is interrupted. It can be called by `nightwatch -n <time> interrupt`. Implemented by reading **"/proc/interrupts"** and parsing it's contents as per requirement.
    1. PID of most recently created process. It can be called by `nightwatch -n <time> newborn`. Implemented by checking time of creation of each process directory in **"/proc/"** using `stat()` and selecting the most recent process.
1. **pinfo**- Displays the process id that was passed with the command, and if one wasn't passed, it displays the process id of the process in which the command was called. Implemented by reading **"/proc/\<pid\>/status"** for that process, parsing it's contents according to requirement and reading the link **"/proc/\<pid\>/exe"** using `readlink()`.
1. **setenv**- Assigns the given value to the environment variable of the given name. If no such variable exists, oen is created. Implemented using `setenv()`.
1. **unsetenv**- Destroys the environment variable with given name. Implemented using `unsetenv()`.
1. **jobs**- Displays all the background processes, their assigned number, status(*Running* or *Stopped*), name and pid. Implemented using the stored queue of background processes and reading **"/proc/\<pid\>/status"** for all processes in the queue.
1. **kjob**- Send the given signal to the given job number in the stored queue of background processes. Implemented using `kill()`.
1. **fg**- Executes the background process with the given job number in the stored queue of background processes as a foreground process. Implemented in the same way *execvp_fg* is implemented.
1. **bg**- Changes the status of the background process with the given job number in the stored queue of background processes to *Running*. Implemented by passing *SIGCONT* to process using `kill()`.
1. **overkill**- Kills all the background processes. Implemented by killing all background processes in the stored queue.
1. **quit**- Exits the shell. Implemented by emptying history buffer, killing all background processes and killing the current process.

## Functionalities Supported

1. **prompt**- Display's command prompt to take take commands from user. Implemented using functions like `getuid()`, `getpwuid()` and `getcwd()`.
1. **status of terminated bg process**- When a background process is terminated, based on how it was terminated, a/an success/error message is displayed. Implemented by handling the signal *SIGCHLD* and functions like `WEXITSTATUS()` and `WIFEXITED()`.
1. **I/O redirection**- Redirect the input and/or output of commands from/to custom files with **"<, >, >>"**. Implemented by duplicating *stdin* and *stdout* file descriptors with the given file descriptors using the function `dup2()`.
1. **piping**- Send output of previous command as input to next command in piped command. This works with redirection **only** for foreground processes. Commands can be piped like **"cmd1 | ... |  cmdn"**. Implementing using the function `pipe()`.
1. **key bindings**- The following key bindings perform their corresponding functions.
    1. ***Ctrl+D***- When there is nothing in the arguments after prompt is displayed, this key binding exits the shell. Implemented by handling *EOF* of *stdin* being returned while taking input of commands.
    1. ***Ctrl+Z***- If a foreground process is running, this key binding pushes it to background. If there is no foreground process running, no task is performed. Implemented by handling the signal *SIGTSTP* and the same  way *execvp_bg* is implemented, if there is a foreground process running.
    1. ***Ctrl+C***- If a foreground process is running, this key binding interrupts it. If there is no foreground process running, no task is performed. Implemented by handling the *SIGINT* signal, and killing the foreground process, if there is one.
1. **feedback on implementation of process**- When a command is executed, if it was successfully executed a **":\`)"** is printed, and if an error occurs a **":\`)"** is printed after the process is executed. Implemented by printing **":\`)"** wherever the command succesfully executes, and printing **":\`)"** wherever a fatal error occurs.

## File Contents Description

1. **values.h**- Stores the values that are used across programs.
1. **headers.h**- Stores all the header files needed to execute the functions in the program.
1. **main.c**- Has the main execution of the shell, signal handling of *SIGTSTP* and *SIGINT*, initialization of values and detection of *quit* command.
1. **prompt.h**- Defines the function that displays the command prompt.
1. **prompt.c**- Has the implementation of the functions defined in *prompt.h*.
1. **parse.h**- Defines the functions that handle piping, redirection and filters arguments.
1. **parse.c**- Has the implementation of the functions defined in *parse.h*.
1. **commands.h**- Defined are functions required for foreground and background process executions, function that finishes background processes and a function that distributes the command as a built-in command, userdefined command or an executable command.
1. **commands.c**- Has the implementation of the functions defined in *commands.h*.
1. **builtin.h**- Defines the functions of the commands *echo*, *pwd*, *cd* and *ls* along with a function to check if the command's execution is using one of these functions and a function that, given the command chooses the function to execute it.
1. **builtin.c**- Has the implementation of the functions defined in *builtin.h*.
1. **userdefined.h**- Defines the functions of the user defined commands *pinfo*, *setenv*, *unsetenv*, *jobs*, *kjob*, *fg*, *bg* and *overkill* along with a function to check if the command's execution is using one of these functions and a function that, given the command chooses the function to execute it.
1. **userdefined.c**- Has the implementation of the functions defined in *userdefined.h*.
1. **history.h**- Defines all functions related to display and storage of the command line history.
1. **history.c**- Has the implementation of the functions defined in *history.h*.

## Assumptions

1. A regular Linux shell does not accept tabs, or **'\t'** character as input.<br>This shell does accept tabs, but discards them while parsing the input.
1. For a space seperated argument, putting a **"\\"** before the space in the argument is the acceptable format while parsing input.
1. Changing directory to previous working directory twice return you to the same directory.
1. Occurence of **"~"** in any argument of a command indicates the directory from which the shell was run.
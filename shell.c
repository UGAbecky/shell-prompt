#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>


/**
 * global variable declaration
 *
 * int arrival is initialized to 0 since we do not want the program to
 * end in a zombie process- when the child process terminates but the
 * parent process has not.
 */
int arrival = 0; //status to see if child finished proccess

/**
 * Command line argument passed to main()
 *
 * we initialize argc to 0 since we take values from the command prompt
 * We start at the command line with 0 commands, then program counts
 * command line arguments when the user provides a desired command.
 *
 * For example, when the user inputs a command, throughout the program
 * the command line arguments(argc) will increase by an increment of 1
 * (argc++). The input is then parsed together.
 */
int argc = 0;


//fork process
pid_t child;

//array of pointers to store
char *arguments[512];

//user's shell buffer
char usersh[512];

//copy of string
char rep[512];

//path to the current directory
char dirPath[512];

//path to user's home directory
char userHome[512];

//user input
char *input;

//displaying the desired directory
char *displayDirectory;
/**
 * Function prototypes used to name the various functions used throughout
 * the program
 *
 * None of the function have a return type or arguments.
 */
void displayPrompt();
void cd();
void replace();
void append();
void redirectIO();



/**
 * Main entry point of the program.
 *
 * Program begins with prompting the user with the shell prompt. The shell
 * prompt prints the text '1730sh' followed by the current working directory,
 * followed by the '$' symbol and a single space.
 *
 * As instructed, the user's prompt will be printed with a tilde symbol
 * representing their home directory.
 */
int main() {

    setbuf(stdout, NULL);

    // forever loop till user specifies exit
    while (1) {

        char *redirStd;
        displayPrompt();
        int n = read(0, usersh, sizeof(usersh));
        usersh[n] = '\0';
        strcpy(rep, usersh);
        input = strtok(usersh, " \t\n<>");

        /**
         * Program checks first if the user has entered a command
         */

        if (!input) {
            continue;
        }

        /**
         * User input for change directory - "cd"
         *
         * User can change to different directory by specifying a path
         * or switch to the parent directory of the current directory.
         */

        else if (strcmp(input, "cd") == 0) {
            cd();
        }
        /**
         * User input for exiting program - "exit"
         *
         * program will terminate normally with clean up procedures
         */
        else if (strcmp(input, "exit") == 0) {
            exit(0);
        }

        /**
         * User has the option to redirect the input and output of the
         * command line.
         *
         * User can specify this by using "<", ">", ">>", and "<<".
         */

        else if ((redirStd = strchr(rep, '<'))) {
            redirectIO();
        }
        else if ((redirStd = strchr(rep, '>'))) {
            //we add +1 to the redirected string to count for offset
            if (*(redirStd + 1) == '>') {
                append();
            }
        }
        else if ((redirStd = strchr(rep, '<'))) {
            if (*(redirStd - 1) == '<') {
                replace();
            } //if*/
        }  else {
            /**
             * execute the program with the command line argument
             * inputted by theh user.
             *
             * The program will first search for an input from the user.
             * if the user enters an invalid command, the program will
             * print an error message.
             */

            child = fork();

         /**
             * When PID == -1, the process cannot be forked
             * Program will throw a perror to notify the user.
             */
            if (child == -1) {
                perror("Message: ERROR process cannot be forked");
                exit(1);
            } //if

    /**
     * when PID == 0, the child proess executes.
     * printf() will be executed in the child process but not
     * in the parent's process.
     */

            else if (child == 0) {
                arguments[argc] = input;

                while (input != NULL) {

                    input = strtok(NULL, " \t\n<>");
                    argc++;
                    arguments[argc] = input;
                } //while


                if ((execvp(arguments[0], arguments) == -1)) {
                    setbuf(stdout, NULL);
                    printf("1730sh: %s: ERROR: please enter valid command\n",
                    usersh);
                } //if

                exit(0);
            } else {
                if (wait(&arrival) == -1) {
                    perror("Message: Error parent process cannot execute");

                } //if
            } // else
        }
    } //while
    return 0;
} //main

/**
 * This function displays the shell prompt. The user will see on their screen
 * the text '1730sh' followed by the current directory they are in, followed by
 * the '$' symbol.
 *
 * If the user already in their home directory, the current directory portion of
 * the prompt will change to a tilde sign (~).
 *
 * If the program cannot retrieve the current directory or the home directory,
 * and error message will display on the screen.
 */

void displayPrompt() {

    //buffer size and letting shell know user should be displayed with prompt
    int workingDir = 1;
    char cwd[512];

    /**
     * when displaying the prompt we want the shell to display the user's home directory
     *
     * getenv() function searches the environment list to find the environment variable name.
     * in our case, we're trying to find the "home" directory so the function returns a pointer
     * to the value specified
     */

    if (workingDir == 1) {
        strcat(dirPath, getenv("HOME"));
        workingDir = 0;
    } //if

    int x = 0;
    while (dirPath[x] != '\0') {
        x++;
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (strncmp(dirPath, cwd, x) == 0 && strcmp(dirPath, cwd) <= 0) {
            /**
             * We want the shell to display the path to the user's home directory
             */
            userHome[0] = '~';
            for (int i = 1; cwd[x] != '\0'; i++, x++)
            {
                userHome[i] = cwd[x];
            }
            /**
             * setbuf(stdout, NULL) will crreate a new buffer in the background.
             */

            setbuf(stdout, NULL);
            printf("1730sh: $ ");
            printf(userHome);

            /**
             * we use memset() to fill a block of memory.
             *
             * in this case, we want the block of memory to be occupied with the
             * memory address of the user's home directory.
             */

            memset(userHome, 0, strlen(userHome));
        } else {
            setbuf(stdout, NULL);
            printf("1730sh:%s$ ", cwd);
            memset(cwd, 0, strlen(cwd));
        }
    } else {
        perror("Message: ERROR with retrieving path to home");
    } // else
} //displayPrompt


/**
 * This function will change the current directory of the user's shell.
 * If the user is currently in a directory, typing in 'cd' into the shell
 * prompt will direct the user into the parent directory.
 *
 * The command 'cd' followed by a valid directory name will direct the user to
 * that directory.
 */

void cd() {
    char *homeKey = getenv("HOME");

    /**
     * Since we specify directories by their set path, we use the function
     * strtok() to extract segmented inputs from the user.
     *
     * the string is then parsed together in str.
     */
    input = strtok(NULL, " \t\n<>");
    if (input == NULL) {
        if (homeKey == NULL) {
            setbuf(stdout, NULL);
            printf("ERROR: environment variable is not specified.\n");
        }
        if (chdir(homeKey) == -1) {
            perror("Message: ERROR cannot change directories");
        } //if
        return;
    } //if

    /**
     * We can represent the user's home directory by the use of a tilde symbol
     *
     * when user's input is ~ at the first argument, it can indicate that the user is
     * already at their home directory or the path to their home is abbreviated.
     */
    if (input[0] == '~') {
        if (homeKey == NULL) {
            setbuf(stdout, NULL);
            printf("ERROR: environment variable is not specified.\n");
            return;
        }
        /**
         * when the cd command is not followed by any path to another directory
         * the path to the home directory is the parent directory.
         */
        char dirName[700];
        strcpy(dirName, homeKey);

        //setting user input to the new directory they put in command line
        displayDirectory = strcat(dirName, &input[1]);
        displayDirectory = input;

    } //if
    if (chdir(input) == -1) {
        perror("Message: ERROR: cannot change directories");
    } //if
} //cd

/**
 * This function redirects the standard output to overwrite another.
 *
 * For an example, this function takes the ouput of Input1.txt and saves
 * the output into another file (Input2.txt)
 *
 * The user can access this input redirection by using the symbol '>'
 */

void replace() {
    char *arguments[512];
    child = fork();

    if (child == -1) {
        perror("Message: ERROR child has failed to execute");
        exit(1);
    }
    else if (child == 0) {
        arguments[argc] = 0;

        /**
         * if the user's input is valid and present
         */
        while (input != NULL)
        {
            input = strtok(NULL, " \t\n<>");
            argc++;
            arguments[argc] = input;
        }

        /**
         * In order to replace one file's contents with the contents of another,
         * we may need to make room for the varying offsets.
         */

        char *originalFile = arguments[argc - 2];
        char *copyFile = arguments[argc - 1];
        int originalFd = open(originalFile, O_RDONLY);
        int copyFd = open(copyFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);


        /**
         * using dup2() we can indicate a target to the new file that is
         * created after appending.
         *
         * in this case, we indicate that the new file will have the file
         * descriptor as the copy file descriptor.
         */
        int n;
        char z;

        dup2(copyFd, STDOUT_FILENO);
        while ((n = read(originalFd, &z, sizeof(z))) > 0) {

            write(copyFd, &z, sizeof(z));
        }

        if (strcmp(input, "exit") == 0) {

            exit(0);
        }
    } //else if
    else {
        if (wait(&arrival) == -1) {
            perror("Error");
            return;
        } //if
    } //else
} //overwrite

/**
 * This function appends a copy of the string.
 * The new string will attach to the end of the current value which
 * will extend the length of the string.
 *
 * The user can type in ">>" to redirect the standard output.
 */

void append() {
    char *arguments[512];
    child = fork();

    if (child == -1) {
        perror("Message: ERROR file cannot be outputted");
        exit(1);
    }

    /**
     * On a successful fork, the child PID returns 0, and the parent's
     * process id returns with the address of the child's.
     *
     * In this case when two valid files are found, we can attach the string
     * of the first file and attach it to the end of the second file.
     */

    else if (child == 0) {
        arguments[argc] = 0;

        /**
         * if the user's input is valid and present,
         * we can parse together two files together.
         *
         * In order to make room for the appended string in the
         * new file, we have to change the file's offset as well.
         */
        while (input != NULL) {
            input = strtok(NULL, " \t\n<>");
            argc++;
            arguments[argc] = input;
        } //while

        char *originalFile = arguments[argc - 2];
        char *copyFile = arguments[argc - 1];

        /**
         * Since we don't need to edit the file descriptor of the
         * original file, the OFlag for the original file
         * is set to read only.
         */
        int ogFd = open(originalFile, O_RDONLY);

        /**
         * In the new file descriptor, we can create a new file
         * we can write, and we can append the file.
         *
         * All the accepted Oflags of open apply to the new file the program
         * made.
         */
        int copyFd = open(copyFile, O_CREAT | O_WRONLY | O_APPEND, 0644);

        int n;
        char z;

        /**
         * using dup2() we can indicate a target fd to the new file that
         * is created after appending
         *
         * in this case we indicate that the new file will have the file
         * descriptor as the copyFile file descriptor.
         */


        dup2(copyFd, STDOUT_FILENO);
        while ((n = read(ogFd, &z, sizeof(z))) > 0) {
            write(copyFd, &z, sizeof(z));
        }

        //exit option for user when appending is finished
        if (strcmp(input, "exit") == 0) {
            exit(0);
        } //if
    } else {
        if (wait(&arrival) == -1) {
            perror("Message: ERROR parent process failed to execute");
            return;
        } //if
    } //else
} //append

/**
 * This function redirects the user's input from the standard input
 *
 * The user can prompt the shell to redirect one file to another file.
 */

void redirectIO() {
    int x;
    int j;
    char *arguments[512];
    child = fork();

    /**
     * Processes failed to launch
     *
     * prompt user with error message
     */

    if (child == -1) {
        perror("Message: ERROR process cannot be forked");
        exit(1);
    } else if (child == 0) {
        arguments[argc] = 0;

        /**
         * user's input is valid and present
         */

        while (input != NULL) {
            input = strtok(NULL, " \t\n<>");
            argc++;
            arguments[argc] = input;
        } // while

        /**
         * creating a pointer for the address of the redirected file
         * we offset the argument by 1 to account for varying byte size
         * of the files
         */
        char *enteredFile = arguments[argc - 1];

        /**
         * Since we are not editting the file, we set the file
         * permissions to read only
         */
        int originalFd = open(enteredFile, O_RDONLY);

        /**
        int x;
        char j;
        */
        /**
         * the function dup2() is used when we want to specify a target as the
         * new file descriptor.
         *
         * the argument for dup2(int old_fd, int new_fd), the old fd is copied into
         * the new fd.
         */
        dup2(1, 1);

        while ((x = read(originalFd, &j, sizeof(j))) > 0) {
            write(1, &j, sizeof(j));
        } //while

        /**
         * user exit prompt
         */

        if (strcmp(input, "exit") == 0) {
            exit(0);
        } //if
    } else {
        /**
         * If parent process fail to execute, user is prompt with an error
         * message
         */
        if (wait(&arrival) == -1) {
            perror(" Message:ERROR parent process failed to execute ");
            return;
        } //if
    } // else
} //redirectIO

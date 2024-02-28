#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

//Allowed functions:
//malloc, free, write, close, fork, waitpid, 
//signal, kill, exit, chdir, execve, dup, dup2, pipe,
//strcmp, strncmp

// QUESTIONS

// 1. Why passing argv[1] to chdir?
//      - because cd just takes 1 argument, 
//      so we know it's going to be the next one at position 1,
//      since pos 0 is the actual string containing the cd command
// 2. Is it necessary to create a child process to execute commands?
//      - no, but in this case it's usefull to wait for its completion and capture its
//      output, so we can pipe this into the next command
// 3. Why setting the element after the pipe or semicolon to null?
//      - because the child executes only until there (only the 1st command)
//      and sends this to fd[1](=write end of pipe), which is from where the parent will read later
// 4. Why using dup2, if old and new fds are the same both in parent and child process?
//      - dup2 is passed 2 args (the last one is the default/old route, the 1st the new route
//      the data has to take) so it redirects the data so that instead of sending this the
//      old route, it sends it to the new file descriptor we specified
//      - child changes the stdout for fd[1], so data is written to fd[1]
//      - parent changes stdin for fd[0], so data is read from fd[0] instead of keyboard input
// 5. Why after execve in child process returning error?
//		- by success, execve doesn't return at all,
//		else we have to print and return the error status
// 6. What would happen in case of multiple pipes?
//

int print_error(char *str)
{
    int i = 0;

    while (str[i])
    {
        write(2, &str[i], 1); //loop writing each char to STDERR --> 2!!
        i++;
    }
    return (1);
}

int cd (char **argv, int i)
{
    if (i < 2)
        return (print_error("error: cd: bad arguments\n"));
	else if (chdir(argv[1]) == -1)
	{
		printf("ERROR TRYING TO EXEC cd AT argv[1] = %s\n", argv[1]);
        return (print_error("error: cd: cannot change directory to "), print_error(argv[1]), print_error("\n"));
	}
	printf("EXEC cd AT argv[1] = %s SUCCESSFULLY\n", argv[1]);
    return (0); //if cmd exec succesfully
}

int my_exec(char **argv, int i, char **envp)
{
    int status = 0;
    int has_pipe = 0;
    int pid = 0;
    int fd[2];

    if (argv[i] && (strcmp(argv[i], "|") == 0)) //if we're at pipe; else we're at ; or END
        has_pipe = 1;
    printf("HAS_PIPE %d\n", has_pipe);
	//........if pipe.......
    if (has_pipe && pipe(fd) == -1) //tries to create a pipe between fd[0](read) && fd[1](write)
        return (print_error("error: fatal\n")); //if pipe creation fails
    //......................
    printf("IF has_pipe TRUE -> PIPE CREATED SUCCESSFULLY\n");
	pid = fork(); //create child process
    if (!pid) //if child -> executes cmd
    {
		printf("...in child process...\n");
        printf("setting ptr at pos %d : %s, to NULL\n", i, argv[i]);
        argv[i] = NULL; //set the element after the pipe or semicolon to null

		//........if pipe.......
		if (has_pipe && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
            return (print_error("error: fatal from child\n")); //err if pipe redir or closing fails
        //......................
		printf("passing ptr at command %s to execve\n", *argv);
		execve(*argv, argv, envp); //exec cmd until argv[i] == NULL
        return (print_error("error: cannot execute "), print_error(*argv), print_error("\n")); //if exec fails, func continues here so we ret error
    }
    waitpid(pid, &status, 0); //parent waits for child to finish (passing status as ptr to change value & so signalize success/failure)
    printf("CHILD RETURNED\n");

	//........if pipe.......
	if (has_pipe && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
        return (print_error("error: fatal from parent\n")); //err if pipe redir or closing fails
    //......................

	return (WIFEXITED(status) && WEXITSTATUS(status)); //checks if prog exited normally/signalled & if exited with code 0(succes)/err code 
}

int main(int argc, char **argv, char **envp)
{
    int i = 0;
    int status = 0;
    
    if (argc > 1)
    {
        while (argv[i] && argv[++i])
        {
			printf("i at %d\n", i);
            //move ptr to i position
            argv += i;
			printf("POINTER MOVED FORWARD TO i : %s\n", argv[0]);
			//i needs to be reset, since ptr it's been moved to i, which now is the new 0 position
			i = 0;
            //iterate until next pipe or semicolon or end of args
            while (argv[i] && strcmp(argv[i], "|") != 0 && strcmp(argv[i], ";") != 0)
            {
			    i++;
				printf("i MOVED TO %d : %s\n", i, argv[i]);
			}
			//execute the 1st cmd argv is pointing to (before pipe, semicolon or end)
            if (strcmp(*argv, "cd") == 0)
                status = cd(argv, i); //execute cd built-in cmd
            else
                status = my_exec(argv, i, envp); //exec other cmds
        }
    }
	printf("RETURNING STATUS %d\n", status);
    return (status);
}
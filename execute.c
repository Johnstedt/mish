#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 

#include "execute.h"

int dupPipe(int pip[2], int end, int destfd)
{
        pid_t childpid;

        if((childpid = fork()) == -1)
        {
        	perror("fork");
                exit(1);
        }

        if(childpid == 0)
        {
        	/* Child process closes up input side of pipe */
                close(pip[0]);

		/* Duplicate the input side of pipe to stdin */
         //       dup(pip[destfd]);

        }
        else
        {
        	/* Parent process closes up output side of pipe */
                close(pip[1]);

		wait(NULL);

        }
	return destfd;
}

int redirect(char *filename, int flags, int destfd)
{



return 1337;

}

/*
 * Systemnära programmering
 * HT15
 * mish

 * Author:	John-John Markstedt<c14jmt@cs.umu.se>
 * Daturm:	19 November 2015
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h> 

#include "parser.h"
#include "execute.h"

void cdFunc(char *s);
void echoFunc(char **s, int n);
void executePrgms(command *command, int pip[][2],int i);

//globals for sigint
pid_t *children; 
int line;

//kills children
static void killChild()
{
	int j;
	for(j = 0; j < line; j++)
	{	
		kill(children[j], SIGKILL);
	}	
}

int main(int argc,char *argv[])
{	
	char s[MAXWORDS];
	command command[MAXCOMMANDS];
	
	//parent set SIGINT function
	struct sigaction sigAct;
	sigAct.sa_handler = killChild;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = SA_RESTART;
	if(sigaction(SIGINT, &sigAct, NULL))
	{
		fprintf(stderr,"Error initiating signal handler: %s", strerror(errno));		
	}

	while(1)
	{	
		//prints prompt
		fprintf(stderr, "mish%% ");
		fflush(stderr);
		int i = 0;
		
		//grabs line from user, if CTRL+D quit.
		if(fgets(s,sizeof(s),stdin) == NULL )
		{
			printf("\n");	
			return 0;
		}

		//parse user input to command
		line = parse(s, command);		
	
		//creates one less pipe than commands
		int pip[line-1][2];
		
		//initalizes the pipes
		for(i = 0;i < line-1; i++)
		{
			pipe(pip[i]);
		}
		
		//allocates memory for global variable
		children = malloc(sizeof(pid_t)*line);

		for(i = 0; i < line; i++ )
		{
			//internal cd
			if(!strcmp(command[i].argv[0], "cd"))
			{		
				cdFunc(command[i].argv[1]);
			}
			//internal echo
			else if(!strcmp(command[i].argv[0], "echo"))
			{						
				echoFunc(command[i].argv, command[i].argc);
			}
			//external command 
			else
			{	
				executePrgms(command,pip, i);
			}			 

		}while(wait(NULL)>0);
		
		//free list of dead children
		free(children);
	}
	return 0;
}

//executes external commands
void executePrgms(command *command,int pip[][2], int i)
{
	pid_t pid[line-1];
	
	//if last command output stdout if no file
	if(i == line -1)
	{
		if((pid[i]= fork())==0)
		{			
			if(command[i].outfile !=NULL)
			{
				FILE *file = fopen(command[i].outfile, "w");
				dup2(fileno(file),WRITE_END);
				fclose(file);
			}
			
			if(command[i].infile !=NULL && i == 0)
			{
				FILE *file = fopen(command[i].infile, "r");
				dup2(fileno(file),READ_END);
				fclose(file);
			}
			else
			{
				dup2(pip[i-1][0],READ_END);
				close(pip[i-1][0]);
				close(pip[i-1][1]);		
			}
			execvp(command[i].argv[0],command[i].argv);
			fprintf(stderr, "%s : %s\n", *command[i].argv, strerror(errno));	
			exit(1);
		}	 
		else
		{	
			close(pip[i-1][0]);                        
			children[i] = pid[i];
		}		
	}
	//first command, reads from stdin instead of pipe
	//if not input file is set
	else if(i == 0)
	{
		if((pid[i] = fork())== 0)
		{	
			if(command[i].infile !=NULL)
			{
				FILE *file = fopen(command[i].infile, "r");
				dup2(fileno(file),READ_END);
				fclose(file);
			}
			
			dup2(pip[i][1],WRITE_END);
			close(pip[i][1]);                        
			close(pip[i][0]);                               
				
			execvp(command[i].argv[0],command[i].argv);	
			fprintf(stderr,"%s : %s\n", *command[i].argv, strerror(errno));
			exit(1);
		}
		else
		{	
			close(pip[i][1]);                        
			children[i] = pid[i];
		}
	}
	//middle commands both read and write to ends of pipe
	else 
	{		
		if((pid[i]= fork())== 0)
		{			
			dup2(pip[i-1][0],READ_END);
			close(pip[i-1][0]);
			close(pip[i-1][1]);		
						
			dup2(pip[i][1], WRITE_END);
			close(pip[i][1]);                        
			close(pip[i][0]);                               
						
			execvp(command[i].argv[0],command[i].argv);	
			fprintf(stderr, "%s : %s\n", *command[i].argv, strerror(errno));
			exit(1);
		}
		else
		{	
			close(pip[i][1]);                     
			close(pip[i-1][0]);
			children[i] = pid[i];
		}
	}
}

//internal command echo
void echoFunc(char **s, int n)
{
	int i = 1;

	for(i = 1;i < n-1;i++)
	{
		printf("%s ", s[i]);	
	}
	printf("%s\n", s[n-1]);

}

//internal command cd
void cdFunc(char *s)
{
	if(-1 == chdir(s))
	{
		fprintf(stderr,"cd: %s : %s\n", s,strerror(errno));
	}
}

#include "parse.h"
#include "headers.h"
#include "commands.h"

int parse(char *buf)
{
	int k=0;
	char *argv[100];
	char *tok = strtok(buf,";");
	while(tok != NULL)
	{
		argv[k++] = tok;
		tok = strtok(NULL,";");
	}
	for(int i=0;i<k;++i)
	{
		int n = strlen(argv[i]);
		int flag = 1;
		for(int j=0;j<n;++j)
		{
			if(argv[i][j] != ' ')
			{
				flag = 0;
				break;
			}
		}
		if(flag)
			continue;
		if(!strncmp(argv[i],"quit",4))
			return 0;
		piping(argv[i]);
	}
}

int piping(char *buf)
{
	int k=0;
	char *pipe_arg[100];
	char *tok = strtok(buf,"|");
	while(tok != NULL)
	{
		pipe_arg[k++] = tok;
		tok = strtok(NULL,"|");
	}
	int pipefiles[2],temp_file = 0,flag = 1;
	if(k == 1)
		return pipe_parse(buf,0);
	for(int i=0;i<k;++i)
	{
		pipe(pipefiles);
		int pid = fork();
		if(pid < 0)
		{
			perror("Error while forking");
			print_error_emoji();
			return 1;
		}
		else if(!pid)
		{
			dup2(temp_file,0);
			if(i < k-1)
				dup2(pipefiles[1],1);
			if(close(pipefiles[0]) < 0)
				perror("Error closing pipeline file");
			if(!pipe_parse(pipe_arg[i],1))
				flag = 0;
			exit(2);
		}
		else
		{
			wait(NULL);
			if(close(pipefiles[1]) < 0)
				perror("Error closing pipeline file");
			temp_file = pipefiles[0];
		}
	}
	return flag;
}

int pipe_parse(char *buf,int piping)
{
	char **inp = (char**)malloc(sizeof(char*)*100);
	for(int i=0;i<100;++i)
		inp[i] = (char*)malloc(sizeof(char)*1000);
	char *cmd, *argv[100], *token;
	int k = 0,n;
	token = strtok(buf," ");
	cmd = token;
	token = strtok(NULL," ");
	while(token != NULL)
	{
		argv[k++] = token;
		token = strtok(NULL," ");
	}
	int cntr = 0,temp = -1;
	for(int i=0;i<100;++i)
		inp[i][0] = '\0';
	for(int z=0;z<k;++z)
	{
		if(temp < 0)
			n = 0;
		else
			n = strlen(inp[temp]);
		int flag = 0;
		int sz = strlen(argv[z]);
		for(int i=0;i<sz;++i)
		{
			if(argv[z][i] != ' ')
				flag = 1;
		}
		if(!flag)
			continue;
		if(n > 0 && inp[temp][n-1] == '\\')
		{
			inp[temp][n-1] = ' ';
			strcat(inp[temp],argv[z]);
		}
		else
			strcpy(inp[++temp],argv[z]);
	}
	k = temp+1;
	for(int i=0;i<k;++i)
	{
		if(inp[i][0] == '~')
		{
			char temp_str[1000];
			strcpy(temp_str,home_dir);
			strcat(temp_str,inp[i]+1);
			strcpy(inp[i],temp_str);
		}
	}
	int x = redirect(cmd,k,inp,piping);
	if(x == 2)
	{
		cmd = NULL;
		for(int i=0;i<100;++i)
		{
			if(inp[i] != NULL)
				free(inp[i]);
		}
		return 0;
	}
	else if(x == 0)
		execute(cmd,k,inp);
	cmd = NULL;
	for(int i=0;i<100;++i)
	{
		if(inp[i] != NULL)
			free(inp[i]);
	}
	free(inp);
	return 1;
}

int min(int a,int b)
{
	if(a < b)
		return a;
	return b;
}

int redirect(char *cmd, int argl, char **argv,int piping)
{
	int bg = 0;
	if(argl > 0)
	{
		int n = strlen(argv[argl-1]);
		if(argv[argl-1][n-1] == '&')
			bg = 1;
	}
	else
	{
		int n = strlen(cmd);
		if(cmd[n-1] == '&')
			bg = 1;
	}
	int type = 0;
	int mi = 100000000,mo = 100000000;
	char *inp_file = "",*out_file = "";
	for(int i=0;i<argl;++i)
	{
		if(!strcmp(argv[i],"<"))
		{
			if(i == argl-1)
			{
				printf("Error: No such file or directory.\n");
				print_error_emoji();
				return 2;
			}
			inp_file = argv[i+1];
			mi = i;
			type |= 1;
			break;
		}
	}
	for(int i=0;i<argl;++i)
	{
		if(!strcmp(argv[i],">"))
		{
			if(i == argl-1)
			{
				printf("Error: No such file or directory.\n");
				print_error_emoji();
				return 2;
			}
			out_file = argv[i+1];
			mo = i;
			type |= 2;
			break;
		}
	}
	if(!(type&2))
	{
		for(int i=0;i<argl;++i)
		{
			if(!strcmp(argv[i],">>"))
			{
				if(i == argl-1)
				{
					printf("Error: No such file or directory.\n");
					print_error_emoji();
					return 2;
				}
				out_file = argv[i+1];
				mo = i;
				type |= 4;
				break;
			}
		}
	}
	argl = min(argl,min(mi,mo));
	int flag = 0;
	for(int i=0;i<100;++i)
	{
		if(!strcmp(cmd,command_list[i]))
			flag = 1;
	}
	if(!type && !piping)
		return 0;
	if(bg)
		strcpy(argv[argl++],"&");
	pid_t pid = fork();
	int stanin = dup(STDOUT_FILENO),stanout = dup(STDIN_FILENO);
	int status;
	if(pid < 0)
	{
		perror("Error in Forking");
		print_error_emoji();
		return 2;
	}
	if(pid == 0)
	{
		if(type&1)
		{
			int fi = open(inp_file, O_RDONLY);
			if(fi < 0)
			{
				perror("Error opening input file");
				print_error_emoji();
				return 2;
			}
			dup2(fi,0);
			if(close(fi) < 0)
				perror("Error closing input file");
		}
		if(type > 1)
		{
			int fo = -1;
			if(type&2)
				fo = open(out_file,O_WRONLY | O_CREAT | O_TRUNC,0644);
			else
				fo = open(out_file,O_WRONLY | O_CREAT | O_APPEND,0644);
			if(fo < 0)
			{
				perror("Error opening output file");
				print_error_emoji();
				return 2;
			}
			dup2(fo,1);
			if(close(fo) < 0)
				perror("Error closing output file");
		}
		if(!flag && !bg)
		{
			char *exec_arg[argl+2];
			exec_arg[0] = (char*)malloc((strlen(cmd)+5)*sizeof(char));
			strcpy(exec_arg[0],cmd);
			for(int i=0;i<argl;++i)
			{
				exec_arg[i+1] = (char*)malloc((strlen(argv[i])+5)*sizeof(char));
				strcpy(exec_arg[i+1],argv[i]);
			}
			exec_arg[argl+1] = NULL;
			if(execvp(exec_arg[0],exec_arg) < 0)
			{
				char err_buf[200];
				sprintf(err_buf,"Error while running %s",exec_arg[0]);
				perror(err_buf);
				print_error_emoji();
				if(kill(getpid(),SIGTERM) < 0)
					kill(getpid(),SIGKILL);
			}
			else
				print_success_emoji();
			for(int i=0;i<argl+2;++i)
			{
				if(exec_arg[i] != NULL)
				{
					free(exec_arg[i]);
					exec_arg[i] = NULL;
				}
			}
		}
		else
			execute(cmd,argl,argv);
		dup2(stanin,0);
		if(close(stanin) < 0)
			perror("Error closing standard input file");
		dup2(stanout,1);
		if(close(stanout) < 0)
			perror("Error closing standard output file");
		exit(2);
	}
	else
		while(wait(&status) != pid);
	return 1;
}
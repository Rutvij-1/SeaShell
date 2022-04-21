#include "headers.h"
#include "builtin.h"
#include "userdefined.h"
#include "commands.h"
#include "history.h"
#include "nightswatch.h"

void shift(int x)
{
	for(int i=x;i<proc_no-1;++i)
	{
		bgjobs[i].argv = bgjobs[i+1].argv;
		bgjobs[i].pid = bgjobs[i+1].pid;
	}
	--proc_no;
}

void fin_proc()
{
	int status,flag = 0;
	pid_t pid = waitpid(-1,&status,WNOHANG);
	char **argv;
	if(pid <= 0)
		return;
	for(int i=0;i<proc_no;++i)
	{
		if(bgjobs[i].pid == pid)
		{
			flag = 1;
			argv = bgjobs[i].argv;
			shift(i);
			break;
		}
	}
	if(!flag)
		return;
	if(!WEXITSTATUS(status) && WIFEXITED(status))
	{
		printf("%s with PID %d exited normally.\n",argv[0],pid);
		print_success_emoji();
	}
	else
	{
		printf("%s with PID %d did not exited normally.\n",argv[0],pid);
		print_error_emoji();
	}
	int k = 0;
	while(argv[k] != NULL)
		++k;
	for(int i=0;i<k;++i)
	{
		if(argv[i] != NULL)
		{
			free(argv[i]);
			argv[i] = NULL;
		}
	}
}

void exec(char *cmd,int argl,char **argv,int bg)
{
	char **exec_arg = (char**)malloc((argl+2)*sizeof(char*));
	exec_arg[0] = (char*)malloc((strlen(cmd)+5)*sizeof(char));
	strcpy(exec_arg[0],cmd);
	for(int i=0;i<argl;++i)
	{
		exec_arg[i+1] = (char*)malloc((strlen(argv[i])+5)*sizeof(char));
		strcpy(exec_arg[i+1],argv[i]);
	}
	exec_arg[argl+1] = NULL;
	int pid = fork();
	if(pid < 0)
	{
		char err_buf[200];
		sprintf(err_buf,"Error while running %s",exec_arg[0]);
		perror(err_buf);
		print_error_emoji();
		return;
	}
	else if(pid == 0)
	{
		setpgid(0,0);
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
	}
	else
	{
		if(bg)
		{
			int flag = 1;
			for(int i=0;i<proc_no;++i)
			{
				if(bgjobs[i].pid == pid)
				{
					flag = 0;
					break;
				}
			}
			if(flag)
			{
				bgjobs[proc_no].argv = exec_arg;
				bgjobs[proc_no].pid = pid;
				++proc_no;
				printf("[%d] %d\n",proc_no,pid);
			}
		}
		else
		{
			curr_pid = pid;
			curr_job_arg = exec_arg;
			signal(SIGTTIN,SIG_IGN);
			signal(SIGTTOU,SIG_IGN);
			if(tcsetpgrp(STDIN_FILENO,pid) < 0)
			{
				char err_buf[200];
				sprintf(err_buf,"Error while running foreground process %s",exec_arg[0]);
				perror(err_buf);
				print_error_emoji();
				if(kill(getpid(),SIGTERM) < 0)
					kill(getpid(),SIGKILL);
			}
			int status;
			pid_t wtpid = waitpid(pid,&status,WUNTRACED);
			tcsetpgrp(STDIN_FILENO,getpgrp());
			signal(SIGTTIN,SIG_DFL);
			signal(SIGTTOU,SIG_DFL);
			if(WIFSTOPPED(status))
			{
				printf("The process %s with process id %d has been suspended.\n",exec_arg[0],pid);
				int flag = 1;
				for(int i=0;i<proc_no;++i)
				{
					if(bgjobs[i].pid == pid)
					{
						flag = 0;
						break;
					}
				}
				if(flag)
				{
					bgjobs[proc_no].argv = exec_arg;
					bgjobs[proc_no].pid = pid;
					++proc_no;
				}
				print_error_emoji();
			}
			else
				print_success_emoji();
		}
	}
}

void execute(char *cmd,int argl,char **argv)
{
	if(in_built_in(cmd))
		exec_built_in(cmd,argl,argv);
	else if(in_user_defined(cmd))
		exec_user_defined(cmd,argl,argv);
	else if(!strcmp(cmd,"history"))
	{
		if(argl == 0)
		{
			if(hist_no < 10)
				print_history(hist_no);
			else
				print_history(10);
		}
		else
		{
			int n;
			sscanf(argv[0],"%d",&n);
			if(n > 10)
				n = 10;
			print_history(n);
		}
	}
	else if(!strcmp(cmd,"nightswatch"))
	{
		if(argl != 3)
		{
			printf("Required output is: nightswatch -n [time_interval] [interrupt/newborn]\n");
			print_error_emoji();
			return;
		}
		if(strcmp(argv[0],"-n"))
		{
			printf("Required output is: nightswatch -n [time_interval] [interrupt/newborn]\n");
			print_error_emoji();
			return;
		}
		int n = 0;
		char str[1000];
		strcpy(str,argv[1]);
		int len = strlen(str);
		for(int i=0;i<len/2;++i)
		{
			char c = str[i];
			str[i] = str[len-1-i];
			str[len-1-i] = c;
		}
		int pw = 1;
		for(int i=0;i<len;++i)
		{
			if(str[i] < '0' && str[i] > '9')
			{
				printf("Required output is: nightswatch -n [time_interval] [interrupt/newborn]\n");
				print_error_emoji();
				return;
			}
			n += pw * (str[i]-'0');
			pw *= 10;
		}
		if(!strcmp(argv[2],"interrupt"))
			interrupt(n);
		else if(!strcmp(argv[2],"newborn"))
			newborn(n);
		else
		{
			printf("Required output is: nightswatch -n [time_interval] [interrupt/newborn]\n");
			print_error_emoji();
			return;
		}
	}
	else
	{
		int bg = 0;
		if(argl > 0)
		{
			int n = strlen(argv[argl-1]);
			if(argv[argl-1][n-1] == '&')
			{
				argv[argl-1][n-1] = '\0';
				if(!strlen(argv[argl-1]))
				{
					--argl;
					free(argv[argl]);
					argv[argl] = NULL;
				}
				bg = 1;
			}
		}
		else
		{
			int n = strlen(cmd);
			if(cmd[n-1] == '&')
			{
				cmd[n-1] = '\0';
				bg = 1;
			}
		}
		exec(cmd,argl,argv,bg);
	}
}
#include "prompt.h"
#include "headers.h"
#include "commands.h"
#include "parse.h"
#include "history.h"
#include "userdefined.h"

void print_success_emoji()
{
	printf("\033[1;32m");
	printf(":`)\n");
	printf("\033[0m");
}

void print_error_emoji()
{
	printf("\033[1;31m");
	printf(":`(\n");
	printf("\033[0m");
}

void Cntr_C()
{
	if(getpid() != ini_pid)
		return;
	if(curr_pid != -1)
	{
		kill(curr_pid,SIGINT);
		print_error_emoji();
	}
}

void Cntr_Z()
{
	if(getpid() != ini_pid)
		return;
	if(curr_pid != -1)
	{
		int flag = 1;
		for(int i=0;i<proc_no;++i)
		{
			if(bgjobs[i].pid == curr_pid)
			{
				flag = 0;
				break;
			}
		}
		if(flag)
		{
			bgjobs[proc_no].argv = curr_job_arg;
			bgjobs[proc_no].pid = curr_pid;
			++proc_no;
		}
		kill(curr_pid,SIGTSTP);
		print_error_emoji();
	}
}

int initialise()
{
	home_dir = getcwd(NULL,0);
	if(home_dir == NULL)
	{
		perror("Error while obtaining home directory");
		return 0;
	}
	proc_no = 0;
	signal(SIGCHLD,fin_proc);
	signal(SIGINT,Cntr_C);
	signal(SIGTSTP,Cntr_Z);
	load_history();
	strcpy(command_list[0],"echo");
	strcpy(command_list[1],"pwd");
	strcpy(command_list[2],"cd");
	strcpy(command_list[3],"ls");
	strcpy(command_list[4],"pinfo");
	strcpy(command_list[5],"history");
	strcpy(command_list[6],"nightswatch");
	strcpy(command_list[7],"setenv");
	strcpy(command_list[8],"unsetenv");
	strcpy(command_list[9],"jobs");
	strcpy(command_list[10],"kjob");
	strcpy(command_list[11],"fg");
	strcpy(command_list[12],"bg");
	strcpy(command_list[13],"overkill");
	for(int i=14;i<100;++i)
		strcpy(command_list[i],"");
	ini_pid = getpid();
	curr_pid = -1;
	curr_job_arg = NULL;
	strcpy(prev_dir,"");
	return 1;
}

int main()
{
	if(!initialise())
	{
		empty_history_buffer();
		return 0;
	}
	while (1)
	{
		char cwd[1000];
		if(getcwd(cwd,1000) == NULL)
		{
			perror("Error while obtaining current working directory");
			empty_history_buffer();
			break;
		}
		if(!prompt(cwd))
		{
			empty_history_buffer();
			break;
		}
		char *buf = NULL;
		int n;
		size_t sz = 0;
		if((n = getline(&buf,&sz,stdin)) == -1)
		{
			empty_history_buffer();
			overkill();
			if(kill(getpid(),SIGTERM) < 0)
				kill(getpid(),SIGKILL);
		}
		if(n > 0)
		{
			buf[n-1] = '\0';
			--n;
		}
		int flag = 1;
		for(int i=0;i<n;++i)
		{
			if(buf[i] != ' ' && buf[i] != '\t')
			{
				flag = 0;
				break;
			}
		}
		if(flag)
			continue;
		add_to_history(buf);
		for(int i=0;i<n;++i)
		{
			if(buf[i] == '\t')
				buf[i] = ' ';
		}
		if(!parse(buf))
		{
			free(buf);
			empty_history_buffer();
			overkill();
			if(kill(getpid(),SIGTERM) < 0)
				kill(getpid(),SIGKILL);
		}
		free(buf);
	}
	return 0;
}

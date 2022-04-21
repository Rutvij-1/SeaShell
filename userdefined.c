#include "headers.h"
#include "userdefined.h"
#include "commands.h"

int in_user_defined(char *cmd)
{
	if(!strcmp(cmd,"pinfo"))
		return 1;
	if(!strcmp(cmd,"setenv"))
		return 1;
	if(!strcmp(cmd,"unsetenv"))
		return 1;
	if(!strcmp(cmd,"jobs"))
		return 1;
	if(!strcmp(cmd,"kjob"))
		return 1;
	if(!strcmp(cmd,"fg"))
		return 1;
	if(!strcmp(cmd,"bg"))
		return 1;
	if(!strcmp(cmd,"overkill"))
		return 1;
	return 0;
}

void pinfo(pid_t pid)
{
	char spid[15],statpath[100],dirpath[100],exepath[1000],finalexepath[1000];
	sprintf(spid,"%d",(int)pid);
	strcat(spid,"/");
	strcpy(dirpath,"/proc/");
	strcat(dirpath,spid);
	strcpy(statpath,dirpath);
	strcat(statpath,"status");
	strcat(dirpath,"exe");
	FILE *f = fopen(statpath,"r");
	if(f == NULL)
	{
		char err_buf[100];
		sprintf(err_buf,"Error getting status and memory for process with PID %d",pid);
		perror(err_buf);
		print_error_emoji();
		return;
	}
	printf("pid -- %d\n",pid);
	char *line = NULL;
	size_t x = 0;
	char proc_size[15],proc_stat[5];
	int len;
	while((len = getline(&line,&x,f)) > 0)
	{
		if(!strncmp(line,"State",5))
		{
			int k = 6,flag = 1;
			while(k < len && (line[k] < 'A' || line[k] > 'Z'))
				++k;
			if(!strncmp(line+k,"R",1))
				strcpy(proc_stat,"R");
			else if(!strncmp(line+k,"Z",1))
				strcpy(proc_stat,"Z");
			else if(!strncmp(line+k,"T",1))
				strcpy(proc_stat,"T");
			else if(!strncmp(line+k,"S+",2))
				strcpy(proc_stat,"S+");			
			else if(!strncmp(line+k,"S",1))
				strcpy(proc_stat,"S");
			else
			{
				fclose(f);
				printf("Error getting status of process with PID %d\n",pid);
				flag = 0;
			}
			if(flag)
				printf("Process Status -- %s\n",proc_stat);
		}
		else if(!strncmp(line,"VmSize",6))
		{
			int k = 0;
			while(k < len && (line[k] < '0' || line[k] > '9'))
				++k;
			strcpy(proc_size,line+k);
			if(proc_size[0] >= '0' && proc_size[0] <= '9')
				printf("Memory -- %s",proc_size);
			else
				printf("Error getting memory of process with PID %d\n",pid);
			break;
		}
		x = 0;
		free(line);
		line = NULL;
	}
	if(fclose(f) != 0)
		printf("Error closing file containing information about the process with PID %d.\n",pid);
	len = readlink(dirpath,exepath,999);
	if(len < 0)
	{
		char err_buf[100];
		sprintf(err_buf,"Error getting the executable path for process with PID %d",pid);
		perror(err_buf);
		print_error_emoji();
		return;
	}
	exepath[len] = '\0';
	len = strlen(home_dir);
	if(!strncmp(exepath,home_dir,len))
	{
		strcpy(finalexepath,"~");
		strcat(finalexepath,exepath+len);
	}
	else
		strcpy(finalexepath,exepath);
	printf("Executable path -- %s\n",finalexepath);
	print_success_emoji();
}

void set_environ(char *varname,char *value)
{
	if(setenv(varname,value,1) < 0)
	{
		char err_buf[200];
		sprintf(err_buf,"Error setting value of %s",varname);
		perror(err_buf);
		print_error_emoji();
	}
	else
		print_success_emoji();
}

void unset_environ(char *varname)
{
	if(unsetenv(varname) < 0)
	{
		char err_buf[200];
		sprintf(err_buf,"Error unsetting %s",varname);
		perror(err_buf);
		print_error_emoji();
	}
	else
		print_success_emoji();
}

void jobs()
{
	int flag = 0;
	if(proc_no == 0)
		flag = 1;
	for(int i=0;i<proc_no;++i)
	{
		printf("[%d] ",i+1);
		char statpath[100] = "";
		sprintf(statpath,"/proc/%d/status",bgjobs[i].pid);
		FILE *f = fopen(statpath,"r");
		if(f == NULL)
		{
			perror("Error getting status of background process");
			continue;
		}
		char *line = NULL;
		size_t x = 0;
		int len;
		int status = -1,found = 0;
		while(!found && (len = getline(&line,&x,f)) > 0)
		{
			if(!strncmp(line,"State",5))
			{
				int k = 6;
				while(k < len && (line[k] < 'A' || line[k] > 'Z'))
					++k;
				if(!strncmp(line+k,"T",1))
					status = 0;
				else
					status = 1;
				found = 1;
			}
			x = 0;
			free(line);
			line = NULL;
		}
		if(status == -1)
		{
			printf("Error getting status of background process\n");
			continue;
		}
		if(status)
			printf("Running ");
		else
			printf("Stopped ");
		printf("%s [%d]\n",bgjobs[i].argv[0],bgjobs[i].pid);
		flag = 1;
	}
	if(flag)
		print_success_emoji();
	else
		print_error_emoji();
}

void kjob(int job_no,int sig_no)
{
	--job_no;
	if(job_no < 0 || job_no >= proc_no)
	{
		printf("Error: Invalid job number\n");
		return;
	}
	if(sig_no < 0 || sig_no > 31 || sig_no == 16)
	{
		printf("Error: Invalid signal number\n");
		return;
	}
	kill(bgjobs[job_no].pid,sig_no);
}

void fg(int job_no)
{
	--job_no;
	if(job_no < 0 || job_no >= proc_no)
	{
		printf("Error: Invalid job number\n");
		print_error_emoji();
		return;
	}
	char **job_argv = bgjobs[job_no].argv;
	curr_job_arg = job_argv;
	int pid = bgjobs[job_no].pid;
	curr_pid = pid;
	shift(job_no);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	tcsetpgrp(STDIN_FILENO,pid);
	kill(pid,SIGCONT);
	int status;
	pid_t wtpid = waitpid(pid,&status,WUNTRACED);
	tcsetpgrp(STDIN_FILENO,getpgrp());
	signal(SIGTTIN,SIG_DFL);
	signal(SIGTTOU,SIG_DFL);
	if(WIFSTOPPED(status))
	{
		printf("The process %s with process id %d has been suspended.\n",job_argv[0],pid);
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
			bgjobs[proc_no].argv = job_argv;
			bgjobs[proc_no].pid = pid;
			++proc_no;
		}
		print_error_emoji();
	}
	else
		print_success_emoji();
}

void bg(int job_no)
{
	--job_no;
	if(job_no < 0 || job_no >= proc_no)
	{
		printf("Error: Invalid job number\n");
		print_error_emoji();
		return;
	}
	kill(bgjobs[job_no].pid,SIGCONT);
}

void overkill()
{
	for(int i=0;i<proc_no;++i)
	{
		if(kill(bgjobs[i].pid,SIGTERM) < 0)
			kill(bgjobs[i].pid,SIGKILL);
	}
	proc_no = 0;
}

void exec_user_defined(char *cmd,int argl,char **argv)
{
	if(!strcmp(cmd,"pinfo"))
	{
		if(argl == 0)
			pinfo(getpid());
		else
		{
			int pid;
			sscanf(argv[0],"%d",&pid);
			pinfo((pid_t)pid);
		}
	}
	else if(!strcmp(cmd,"setenv"))
	{
		if(argl == 0)
		{
			printf("Error: Insuffiecient arguments.\n Expected format is: setenv [var name] [value]\n");
			print_error_emoji();
			return;
		}
		else if(argl > 2)
		{
			printf("Error: Too many arguments.\n Expected format is: setenv [var name] [value]\n");
			print_error_emoji();
			return;
		}
		if(argl == 1)
			strcpy(argv[1],"");
		set_environ(argv[0],argv[1]);
	}
	else if(!strcmp(cmd,"unsetenv"))
	{
		if(argl == 0)
		{
			printf("Error: Insuffiecient arguments.\n Expected format is: unsetenv [var name]\n");
			print_error_emoji();
			return;
		}
		else if(argl > 1)
		{
			printf("Error: Too many arguments.\n Expected format is: unsetenv [var name]\n");
			print_error_emoji();
			return;
		}
		unset_environ(argv[0]);
	}
	else if(!strcmp(cmd,"jobs"))
	{
		if(argl > 0)
		{
			printf("Error: Too many arguments.\n Expected format is: jobs\n");
			print_error_emoji();
			return;
		}
		jobs();
	}
	else if(!strcmp(cmd,"kjob"))
	{
		if(argl < 2)
		{
			printf("Error: Insuffiecient arguments.\n Expected format is: kjob [job number] [signal number]\n");
			print_error_emoji();
			return;
		}
		else if(argl > 2)
		{
			printf("Error: Too many arguments.\n Expected format is: kjob [job number] [signal number]\n");
			print_error_emoji();
			return;
		}
		int job_no = -1,sig_no = -1;
		sscanf(argv[0],"%d",&job_no);
		sscanf(argv[1],"%d",&sig_no);
		kjob(job_no,sig_no);
	}
	else if(!strcmp(cmd,"fg"))
	{
		if(argl == 0)
		{
			printf("Error: Insuffiecient arguments.\n Expected format is: fg [job number]\n");
			print_error_emoji();
			return;
		}
		else if(argl > 1)
		{
			printf("Error: Too many arguments.\n Expected format is: fg [job number]\n");
			print_error_emoji();
			return;
		}
		int job_no = -1;
		sscanf(argv[0],"%d",&job_no);
		fg(job_no);
	}
	else if(!strcmp(cmd,"bg"))
	{
		if(argl == 0)
		{
			printf("Error: Insuffiecient arguments.\n Expected format is: bg [job number]\n");
			print_error_emoji();
			return;
		}
		else if(argl > 1)
		{
			printf("Error: Too many arguments.\n Expected format is: bg [job number]\n");
			print_error_emoji();
			return;
		}
		int job_no = -1;
		sscanf(argv[0],"%d",&job_no);
		bg(job_no);
	}
	else if(!strcmp(cmd,"overkill"))
	{
		if(argl > 0)
		{
			printf("Error: Too many arguments.\n Expected format is: overkill\n");
			print_error_emoji();
			return;
		}
		overkill();
	}
}
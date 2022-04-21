#include "headers.h"
#include "nightswatch.h"

int get_cpu_names()
{
	FILE *in = fopen("/proc/interrupts","r");
	if(in == NULL)
	{
		perror("Error opening /proc/interrupts");
		print_error_emoji();
		return 0;
	}
	char **cpu = (char**)malloc(20*sizeof(char*)),*line = NULL;
	int k = 0;
	size_t x = 0;
	int len = getline(&line,&x,in);
	if(len <= 0)
	{
		printf("Error reading /proc/interrupts\n");
		print_error_emoji();
		return 0;
	}
	line[len-1] = '\0';
	--len;
	for(int i=0;i<len;++i)
	{
		if(line[i] == '\t')
			line[i] = ' ';
	}
	char *token = strtok(line," ");
	cpu[k++] = token;
	while((token = strtok(NULL," ")) != NULL)
		cpu[k++] = token;
	free(line);
	return k;
}

int get_interrupt_values(long long itrp[],int n)
{
	char *line = NULL,*rd[50];
	int k = 0;
	size_t x = 0;
	FILE *in = fopen("/proc/interrupts","r");
	if(in == NULL)
	{
		perror("Error opening /proc/interrupts");
		print_error_emoji();
		return 0;
	}
	int len = getline(&line,&x,in);
	if(len <= 0)
	{
		printf("Error reading /proc/interrupts\n");
		print_error_emoji();
		return 0;
	}
	free(line);
	line = NULL;
	x = 0;
	len = getline(&line,&x,in);
	if(len <= 0)
	{
		printf("Error reading /proc/interrupts\n");
		print_error_emoji();
		return 0;
	}
	free(line);
	line = NULL;
	x = 0;
	len = getline(&line,&x,in);
	if(len <= 0)
	{
		printf("Error reading /proc/interrupts\n");
		print_error_emoji();
		return 0;
	}
	line[len-1] = '\0';
	--len;
	for(int i=0;i<len;++i)
	{
		if(line[i] == '\t')
			line[i] = ' ';
	}
	char *token = strtok(line," ");
	rd[k++] = token;
	while((token = strtok(NULL," ")) != NULL)
		rd[k++] = token;
	free(line);
	for(int i=0;i<n;++i)
		sscanf(rd[i+1],"%lld",&itrp[i]);
	return 1;
}

void interrupt(int t)
{
	int n = get_cpu_names();
	char cpu[20][10];
	if(n == 0)
		return;
	for(int i=0;i<n;++i)
		sprintf(cpu[i],"cpu%d",i);
	for(int i=0;i<n;++i)
		printf(" %16s",cpu[i]);
	printf("\n");
	int pid = fork();
	if(pid < 0)
	{
		perror("Error executing nightswatch");
		return;
	}
	else if(pid == 0)
	{
		long long val[n];
		while(1)
		{
			if(!get_interrupt_values(val,n))
			{
				sleep(t);
				continue;
			}
			for(int i=0;i<n;++i)
				printf(" %16lld",val[i]);
			print_success_emoji();
			printf("\n");
			sleep(t);
		}
	}
	else
	{
		while(1)
		{
			char *buf = NULL;
			size_t x = 0;
			int len = getline(&buf,&x,stdin);
			int q = 0,o = 0;
			for(int i=0;i<len;++i)
			{
				if(!q && buf[i] == 'q')
					q = 1;
				else if(buf[i] != ' ' && buf[i] != '\t' && buf[i] != '\n')
					o = 1;
			}
			free(buf);
			if(q && !o)
			{
				if(kill(pid,SIGTERM) < 0)
					kill(pid,SIGKILL);
				break;
			}
		}
	}
}

int get_new_pid()
{
	struct dirent *de;
	DIR *dir = opendir("/proc/");
	if(dir == NULL)
	{
		perror("Error executing nightswatch");
		print_error_emoji();
		return 0;
	}
	char pid[15] = "0",path[30];
	int max_time = -1;
	while((de = readdir(dir)) != NULL)
	{
		strcpy(path,de->d_name);
		int len = strlen(path);
		int flag = 0;
		for(int i=0;i<len;++i)
		{
			if(path[i] < '0' || path[i] > '9')
				flag = 1;
		}
		if(flag)
			continue;
		strcpy(path,"/proc/");
		strcat(path,de->d_name);
		struct stat sb;
		if(stat(path,&sb) == -1)
		{
			perror("Error executing nightswatch");
			print_error_emoji();
			return 0;
		}
		if(!S_ISDIR(sb.st_mode))
			continue;
		if((int)sb.st_ctim.tv_sec > max_time)
		{
			max_time = sb.st_ctim.tv_sec;
			strcpy(pid,de->d_name);
		}
	}
	closedir(dir);
	int n = 0;
	sscanf(pid,"%d",&n);
	return n;
}

void newborn(int t)
{
	int pid = fork();
	if(pid < 0)
	{
		perror("Error executing nightswatch");
		print_error_emoji();
		return;
	}
	else if(pid == 0)
	{
		while(1)
		{
			int n;
			if((n = get_new_pid()) == 0)
			{
				sleep(t);
				continue;
			}
			printf("%d\n",n);
			print_success_emoji();
			sleep(t);
		}
	}
	else
	{
		while(1)
		{
			char *buf = NULL;
			size_t x = 0;
			int len = getline(&buf,&x,stdin);
			int q = 0,o = 0;
			for(int i=0;i<len;++i)
			{
				if(!q && buf[i] == 'q')
					q = 1;
				else if(buf[i] != ' ' && buf[i] != '\t' && buf[i] != '\n')
					o = 1;
			}
			free(buf);
			if(q && !o)
			{
				if(kill(pid,SIGTERM) < 0)
					kill(pid,SIGKILL);
				break;
			}
		}
	}
}

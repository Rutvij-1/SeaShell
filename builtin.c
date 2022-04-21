#include "headers.h"
#include "builtin.h"

int in_built_in(char *cmd)
{
	if(!strcmp(cmd,"echo"))
		return 1;
	if(!strcmp(cmd,"pwd"))
		return 1;
	if(!strcmp(cmd,"cd"))
		return 1;
	if(!strcmp(cmd,"ls"))
		return 1;
	return 0;
}

void echo(char *msg)
{
	printf("%s\n",msg);
	print_success_emoji();
}

void pwd()
{
	char pwd[1000];
	if(getcwd(pwd,1000) == NULL)
	{
		perror("Error getting current working directory");
		print_error_emoji();
		return;
	}
	printf("%s\n",pwd);
	print_success_emoji();
}

void cd(char *path)
{
	char pwd[1000];
	if(getcwd(pwd,1000) == NULL)
	{
		perror("Error getting current working directory");
		print_error_emoji();
		return;
	}
	if(!strcmp(path,"-"))
	{
		if(!strlen(prev_dir))
			return;
		strcpy(path,prev_dir);
		char print_path[1000];
		if(!strncmp(path,home_dir,strlen(home_dir)))
		{
			strcpy(print_path,"~");
			strcat(print_path,path+strlen(home_dir));
		}
		else
			strcpy(print_path,path);
		if(chdir(path) == -1)
		{
			perror("Error occurred while changing directory");
			print_error_emoji();
			return;
		}
		printf("%s\n",print_path);
	}
	else
	{
		if(chdir(path) == -1)
		{
			perror("Error occurred while changing directory");
			print_error_emoji();
			return;
		}
	}
	strcpy(prev_dir,pwd);
	print_success_emoji();
}

void ls(int fl,int fa,char *dir_path)
{
	struct dirent *f;
	DIR *dr = opendir(dir_path);
	if(dr == NULL)
	{
		perror("Could not open directory");
		print_error_emoji();
		return;
	}
	char fname[1000];
	while((f = readdir(dr)) != NULL)
	{
		strcpy(fname,f->d_name);
		if(!fa && fname[0] == '.')
			continue;
		if(fl)
		{
			static char file_path[1000];
			strcpy(file_path,dir_path);
			strcat(file_path,"/");
			strcat(file_path,fname);
			struct stat sb;
			if(stat(file_path,&sb) == -1)
			{
				char err_buf[1100];
				sprintf(err_buf,"Error reading %s",fname);
				perror(err_buf);
				continue;
			}
			if(S_ISDIR(sb.st_mode))
				printf("d");
			else
				printf("-");
			if(sb.st_mode & S_IRUSR)
				printf("r");
			else
				printf("-");
			if(sb.st_mode & S_IWUSR)
				printf("w");
			else
				printf("-");
			if(sb.st_mode & S_IXUSR)
				printf("x");
			else
				printf("-");
			if(sb.st_mode & S_IRGRP)
				printf("r");
			else
				printf("-");
			if(sb.st_mode & S_IWGRP)
				printf("w");
			else
				printf("-");
			if(sb.st_mode & S_IXGRP)
				printf("x");
			else
				printf("-");
			if(sb.st_mode & S_IROTH)
				printf("r");
			else
				printf("-");
			if(sb.st_mode & S_IWOTH)
				printf("w");
			else
				printf("-");
			if(sb.st_mode & S_IXOTH)
				printf("x");
			else
				printf("-");
			printf("  %3d",(int)sb.st_nlink);
			struct passwd *pws;
			pws = getpwuid(sb.st_uid);
			if(pws == NULL)
			{
				char err_buf[1100];
				sprintf(err_buf,"\rError getting owner name of %s\t\t\t",fname);
				perror(err_buf);
				continue;
			}
			printf("  %10s",pws->pw_name);
			struct group *grp;
			grp = getgrgid(sb.st_gid);
			if(grp == NULL)
			{
				char err_buf[1100];
				sprintf(err_buf,"\rError getting group name of %s\t\t\t",fname);
				perror(err_buf);
				continue;
			}
			printf("  %10s",grp->gr_name);
			printf("  %10d",(int)sb.st_size);
			struct tm *tme = localtime(&sb.st_mtim.tv_sec);
			switch(tme->tm_mon)
			{
				case 0:
				{
					printf("  Jan");
					break;
				}
				case 1:
				{
					printf("  Feb");
					break;
				}
				case 2:
				{
					printf("  Mar");
					break;
				}
				case 3:
				{
					printf("  Apr");
					break;
				}
				case 4:
				{
					printf("  May");
					break;
				}
				case 5:
				{
					printf("  Jun");
					break;
				}
				case 6:
				{
					printf("  Jul");
					break;
				}
				case 7:
				{
					printf("  Aug");
					break;
				}
				case 8:
				{
					printf("  Sep");
					break;
				}
				case 9:
				{
					printf("  Oct");
					break;
				}
				case 10:
				{
					printf("  Nov");
					break;
				}
				case 11:
				{
					printf("  Dec");
					break;
				}
				default:
				{
					printf("     ");
					break;
				}
			}
			printf(" %2d",tme->tm_mday);
			printf(" %d",1900+tme->tm_year);
			printf(" %0*d:%0*d",2,tme->tm_hour,2,tme->tm_min);
			printf("  %s\n",fname);
		}
		else
			printf("%s\n",fname);
	}
	closedir(dr);
	print_success_emoji();
}

void exec_built_in(char *cmd,int argl,char **argv)
{
	if(!strcmp(cmd,"echo"))
	{
		char msg[1000];
		if(argl > 0)
		{
			strcpy(msg,argv[0]);
			for(int i=1;i<argl;++i)
			{
				strcat(msg," ");
				strcat(msg,argv[i]);
			}
		}
		if(argl > 0)
			echo(msg);
		else
			echo("");
	}
	else if(!strcmp(cmd,"pwd"))
		pwd();
	else if(!strcmp(cmd,"cd"))
	{
		if(argl == 1)
			cd(argv[0]);
		else if(argl == 0)
		{
			printf("Insufficient arguments.\n");
			print_error_emoji();
		}
		else
		{
			printf("Too many arguments.\n");
			print_error_emoji();
		}
	}
	else if(!strcmp(cmd,"ls"))
	{
		char path[1000];
		int fl = 0,fa = 0,flag = 0;
		for(int i=0;i<argl;++i)
		{
			if(argv[i][0] == '-')
			{
				if((strlen(argv[i]) == 1) ||
					(strcmp(argv[i],"-l") && strcmp(argv[i],"-a") && strcmp(argv[i],"-al") && strcmp(argv[i],"-la")))
				{
					printf("Invalid Flag.\n");
					print_error_emoji();
					return;
				}
				if(argv[i][1] == 'l')
					fl = 1;
				else
					fa = 1;
				if(strlen(argv[i]) > 2)
				{
					if(argv[i][2] == 'l')
						fl = 1;
					else
						fa = 1;
				}
			}
			else
				++flag;
		}
		if(flag)
		{
			int cntr = flag;
			if(flag > 1)
				flag = 1;
			for(int i=0;i<argl;++i)
			{
				if(argv[i][0] != '-')
				{
					strcpy(path,argv[i]);
					if(flag)
					{
						char print_path[1000];
						if(!strncmp(path,home_dir,strlen(home_dir)))
							printf("~%s:\n",path+strlen(home_dir));
						else
							printf("%s:\n",path);
					}
					ls(fl,fa,path);
					--cntr;
					if(cntr)
						printf("\n");
				}
			}
		}
		else
		{
			char cwd[1000];
			if(getcwd(cwd,1000) == NULL)
			{
				perror("Error getting current working directory");
				print_error_emoji();
				return;
			}
			ls(fl,fa,cwd);
		}
	}
}
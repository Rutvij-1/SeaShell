#include "prompt.h"
#include "headers.h"
#include <sys/utsname.h>

int prompt(char* cwd)
{
	int uid = getuid();
	struct passwd *user = getpwuid(uid);
	if(user == NULL)
	{
		perror("Error occurred while obtaining username");
		return 0;
	}
	char *username = user->pw_name;
	char host[1000];
	if(gethostname(host,1000) == -1)
	{
		perror("Error occurred while obtaining host name");
		return 0;
	}
	char* dir_path = (char*)malloc(strlen(cwd)*sizeof(char));
	strcpy(dir_path,cwd);
	int len = strlen(home_dir);
	if(!strncmp(home_dir,cwd,len))
	{
		dir_path[len-1] = '~';
		dir_path += len-1;
	}
	printf("\033[1;36m");
	printf("<%s@%s:%s> ",username,host,dir_path);
	printf("\033[0m");
	return 1;
}

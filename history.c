#include "headers.h"
#include "history.h"

void load_history()
{
	hist_no = 0;
	char history_file[1000];
	strcpy(history_file,home_dir);
	strcat(history_file,"/history.txt");
	FILE *h = fopen(history_file,"r");
	if(h == NULL)
		return;
	char *line = NULL;
	size_t x = 0;
	int len;
	len = getline(&line,&x,h);
	while(len > 0 && hist_no < 20)
	{
		hist[hist_no] = (char*)malloc((len+5)*sizeof(char));
		strcpy(hist[hist_no],line);
		++hist_no;
		if(line != NULL)
			free(line);
		line = NULL;
		len = getline(&line,&x,h);
	}
	fclose(h);
}

void update_history()
{
	char history_file[1000];
	strcpy(history_file,home_dir);
	strcat(history_file,"/history.txt");
	FILE *h = fopen(history_file,"w");
	if(h == NULL)
	{
		perror("Error opening/creating history.txt");
		return;
	}
	for(int i=0;i<hist_no;++i)
	{
		if(fprintf(h,"%s",hist[i]) < 0)
		{
			printf("Error updating history.txt\n");
			return;
		}
	}
	fclose(h);
}

void print_history(int n)
{
	if(hist_no < n)
		n = hist_no;
	for(int i=hist_no-n;i<hist_no;++i)
		printf("%s",hist[i]);
	if(n > 0)
		print_success_emoji();
}

void add_to_history(char *buf)
{
	if(hist_no < 20)
	{
		hist[hist_no] = (char*)malloc((strlen(buf)+5)*sizeof(char));
		strcpy(hist[hist_no],buf);
		strcat(hist[hist_no],"\n");
		++hist_no;
	}
	else
	{
		free(hist[0]);
		for(int i=1;i<20;++i)
			hist[i-1] = hist[i];
		hist[19] = (char*)malloc((strlen(buf)+5)*sizeof(char));
		strcpy(hist[19],buf);
		strcat(hist[19],"\n");
	}
	update_history();
}

void empty_history_buffer()
{
	update_history();
	for(int i=0;i<hist_no;++i)
	{
		if(hist[i] != NULL)
		{
			free(hist[i]);
			hist[i] = NULL;
		}
	}
	hist_no = 0;
}

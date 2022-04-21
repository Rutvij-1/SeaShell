char *home_dir;

extern int errno;

struct BG_proc
{
	char **argv;
	pid_t pid;
};

struct BG_proc bgjobs[1000];

int proc_no;

char *hist[20];

int hist_no;

char command_list[100][100];

int ini_pid,curr_pid;

char **curr_job_arg;

char prev_dir[1000];
int in_user_defined(char *cmd);

void pinfo(pid_t pid);

void set_environ(char *varname,char *value);

void unset_environ(char *varname);

void jobs();

void kjob(int job_no,int sig_no);

void fg(int job_no);

void bg(int job_no);

void overkill();

void exec_user_defined(char *cmd,int argl,char **argv);

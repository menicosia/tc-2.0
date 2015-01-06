struct event *tc2_q_read(struct tcds *t) ;
int tc2_read(struct tcds *t, struct event *e) ;
int pop_all(struct tcds *t, struct user *u) ;
int proc_line(struct tcds *t, struct user *u, char *str) ;
int proc_cmd(struct tcds *t, struct user *u, char *name) ;
int announce_new(struct tcds *t, struct user *u) ;
